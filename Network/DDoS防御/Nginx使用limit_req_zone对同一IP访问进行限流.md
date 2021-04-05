# Nginx使用limit_req_zone对同一IP访问进行限流

src:https://blog.csdn.net/keketrtr/article/details/75315330



nginx可以使用ngx_http_limit_req_module模块的limit_req_zone指令进行限流访问，防止用户恶意攻击刷爆服务器。ngx_http_limit_req_module模块是nginx默认安装的，所以直接配置即可。

**首先，在nginx.conf文件中的http模块下配置**

`limit_req_zone $binary_remote_addr zone=one:10m rate=1r/s;`
说明：区域名称为one（自定义），占用空间大小为10m，平均处理的请求频率不能超过每秒一次。
`$binary_remote_addr`是`$remote_addr`（客户端IP）的二进制格式，固定占用4个字节（可能是C语言的long类型长度）。而`$remote_addr`按照字符串存储，占用7-15个字节。这样看来用`$binary_remote_addr`可以节省空间，但网上又说64位系统下都是占用64个字节，没搞清楚，总之尽量用`$binary_remote_addr`吧。

**第二，在http模块的子模块server下面配置**

```
location ~* .htm$ {
limit_req zone=one burst=5  nodelay;
proxy_pass http://backend_tomcat;
}
```

我这里是对uri后缀为htm的请求限流，注意limit_req zone=one burst=5  nodelay;

其中zone=one和前面的定义对应。

burst这个网上都说峰值之类的，通过亲自试验发现这么说并不准确，应该叫缓冲队列的长度比较合适。

nodelay字面的意思是不延迟，具体说是对用户发起的请求不做延迟处理，而是立即处理。比如我上面定义的rate=1r/s，即每秒钟只处理1个请求。如果同一时刻有两个后缀为htm的请求过来了，若设置了nodelay，则会立刻处理这两个请求。若没设置nodelay，则会严格执行rate=1r/s的配置，即只处理一个请求，然后下一秒钟再处理另外一个请求。直观的看就是页面数据卡了，过了一秒后才加载出来。


**真正对限流起作用的配置就是rate=1r/s和burst=5这两个配置。下面我们来分析一下具体案例。**


某一时刻有两个请求同时到达nginx，其中一个被处理，另一个放到了缓冲队列里。虽然配置了nodelay导致第二个请求也被瞬间处理了，但还是占用了缓冲队列的一个长度，如果下一秒没有请求过来，这个占用burst一个长度的空间就会被释放，否则就只能继续占用着burst的空间，直到burst空间占用超过5之后，再来请求就会直接被nginx拒绝，返回503错误码。

可见，如果第二秒又来了两个请求，其中一个请求又占用了一个burst空间，第三秒、第四秒直到第五秒，每秒都有两个请求过来，虽然两个请求都被处理了（因为配置了nodelay），但其中一个请求仍然占用了一个burst长度，五秒后整个burst长度=5都被占用了。第六秒再过来两个请求，其中一个请求就被拒绝了。


这是我根据实际测试结果推论的，可能和真实的理论有所出入，但这样讲我觉得比较好理解。有清楚的朋友欢迎告知！


**这里用到的`$binary_remote_addr`是在客户端和nginx之间没有代理层的情况。如果你在nginx之前配置了CDN，那么`$binary_remote_addr`的值就是CDN的IP地址。这样限流的话就不对了。需要获取到用户的真实IP进行限流。**

简单说明如下：

**这里取得原始用户的IP地址**

```
map $http_x_forwarded_for  $clientRealIp {
"" $remote_addr;
~^(?P<firstAddr>[0-9\.]+),?.*$$firstAddr;
}
```

**针对原始用户 IP 地址做限制**

```
limit_req_zone $clientRealIp zone=one:10m  rate=1r/s;
```

同理，我们可以用limit模块对网络爬虫进行限流。

http模块

```
limit_req_zone $anti_spider zone=anti_spider:10m rate=1r/s;
```

server模块

```
location / {
limit_req zone=anti_spider burst=2 nodelay;
if ($http_user_agent ~* "spider|Googlebot") {
set $anti_spider $http_user_agent;
}
}
```

可以用`curl -I -A "Baiduspider" www.remotejob.cn/notice.jsp` 测试一下