# 浅谈“慢速HTTP攻击Slow HTTP Attack”

src:https://blog.csdn.net/weixin_39934520/article/details/107707268



# 1. 漏洞名称

>Slow Http attack、慢速攻击

# 2. 描述

HTTP慢速攻击也叫slow http attack，是一种DoS攻击的方式。由于HTTP请求底层使用TCP网络连接进行会话，因此如果中间件对会话超时时间设置不合理，并且HTTP在发送请求的时候采用慢速发HTTP请求，就会导致占用一个HTTP连接会话。如果发送大量慢速的HTTP包就会导致拒绝服务攻击DoS。

Slow Attack 大致可分为以下几种：

* `Slow headers(也称slowloris)`：每个 HTTP 请求都是以空行结尾，即以两个 (\r\n)结 尾 。 若将空行去掉 ,即以 一个 (\r\n) 结尾,则服务器会一直等待直到超时。在等待过程中占用线程（连接数），服务器线程数量达到极限，则无法处理新的合法的 HTTP请求，达到DOS目的。

* `Slow read(也称Slow Read attack)`：向 Web 服务器发送正常合法的 read 请求，请求一个很大的文件，并将 TCP 滑动窗口 size 设置很小如 1 或 2，服务器就会以非常缓慢的速度发送文件，文件将长期滞留在服务器内存中，消耗资源，造成DOS。

* `Slow body(也称Slow HTTP POST)`：攻击者向服务器发送 POST 请求,告诉服务器它将要 POST 的数据为 n,服务器将分配长度为 n 的空间来等待接收数据。当 n 足够大, POST 的请求足够多的时候,这种攻击会占用服务器的大量内存,从而降低服务器性能,甚至导致瘫痪。

* 以及多年前的 `Apache Range Attack`（现已修复）：在 HTTP 请求的 RANGE HEADER 中包含大量字段,使得服务器在服务端将一个很小的文件分割成大量的更小的片段再压缩。分段压缩过程消耗大量的服务器资源,导致 DOS。


这里有两点要注意：

1. tcp窗口设置要比服务器的socket缓存小，这样发送才慢。
2. 请求的文件要比服务器的socket缓存大，使得服务器无法一下子将文件放到缓存，然后去处理其他事情，而是必须不停的将文件切割成窗口大小，再放入缓存。同时攻击端一直说自己收不到。

# 3. 检测条件
>Web业务运行正常

# 4. 检测方法
## 使用工具：slowhttptest
[slowhttptest](https://github.com/shekyan/slowhttptest)是一款对服务器进行慢攻击的测试软件，所谓的慢攻击就是相对于cc或者DDoS的快而言的，并不是只有量大速度快才能把服务器搞挂，使用慢攻击有时候也能到达同一效果。slowhttptest包含了之前几种慢攻击的攻击方式，包括slowloris, Slow HTTP POST, Slow Read attack等。那么这些慢攻击工具的原理就是想办法让服务器等待，当服务器在保持连接等待时，自然就消耗了资源。

## 参考命令
slowloris模式：

```
slowhttptest -c 1000 -H -i 10 -r 200 -t GET -u https://yourtarget.com/index.html -x 24 -p 3
```

Slow Body攻击：

```
slowhttptest -c 1000 -B -g -o my_body_stats -i 110 -r 200 -s 8192 -t FAKEVERB -u http://www.mywebsite.com -x 10 -p 3
```

Slow Read模式：

```
slowhttptest -c 1000 -X -r 1000 -w 10 -y 20 -n 5 -z 32 -u http://yourtarget.com -p 5 -l 350 -e x.x.x.x:8080
```

## 判断依据
1. 当服务器可控，可以通过以下命令来确认是否存在该漏洞：

	pgrep http | wc -l  进程数量

	netstat -antp | grep 443 |wc -l  网络连接数量

2. 在攻击的时间段，服务无法正常访问则存在漏洞。
	
	具体的漏洞浮现，自行百度，这里暂不提供。

# 5. 漏洞修复
1. 将标题和消息体限制在最小的合理长度上。针对接受数据的每个资源，设置更严格的特定于URL的限制。

2. 设置合理的连接超时时间

3. 定义最小传入数据速率，并删除比该速率慢的连接

4. 如果Web服务器从相同的IP接收到数千个连接，同一个用户代理在短时间内请求相同的资源，直接禁掉IP并且记录日志

 5. 对web服务器的http头部传输的最大许可时间进行限制，修改成最大许可时间为20秒。

# 6. 其他补充说明
参考链接：
<http://www.bubuko.com/infodetail-1945145.html>

<https://www.pianshen.com/article/502565992/>

<https://cloud.tencent.com/developer/article/1180216>

<https://www.cnblogs.com/endust/p/11960901.html>