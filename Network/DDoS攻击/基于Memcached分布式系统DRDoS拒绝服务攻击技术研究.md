# 基于Memcached分布式系统DRDoS拒绝服务攻击技术研究

src:https://blog.csdn.net/microzone/article/details/79262549?from=timeline&isappinstalled=0

#### 一、基础概念

*友情提醒：以下仅做技术研究，如有恶意利用，发起攻击，将自负法律后果

本次反射式拒绝服务攻击技术基于全球互联网分布式的Memcached服务器，需要储备一定得安全攻防知识，网络协议知识和python代码编程技术。希望在学习本篇文章知识前自行学习相关的基础知识，文章后面同时附有参考链接。

##### 关于Memcached系统

Memcached是一个自由开源的，高性能，分布式内存对象缓存系统。Memcached是以LiveJournal旗下Danga Interactive公司的Brad Fitzpatric为首开发的一款软件。现在已成为mixi、hatena、Facebook、Vox、LiveJournal等众多服务中提高Web应用扩展性的重要因素。Memcached是一种基于内存的key-value存储，用来存储小块的任意数据（字符串、对象）。这些数据可以是数据库调用、API调用或者是页面渲染的结果。Memcached简洁而强大。它的简洁设计便于快速开发，减轻开发难度，解决了大数据量缓存的很多问题。它的API兼容大部分流行的开发语言。本质上，它是一个简洁的key-value存储系统。一般的使用目的是，通过缓存数据库查询结果，减少数据库访问次数，以提高动态Web应用的速度、提高可扩展性。

##### 关于分布式DDoS原理

分布式拒绝服务(DDoS:Distributed Denial of Service)攻击指借助于客户/服务器技术，将多个计算机联合起来作为攻击平台，对一个或多个目标发动DDoS攻击，从而成倍地提高拒绝服务攻击的威力。通常，攻击者使用一个偷窃帐号将DDoS主控程序安装在一个计算机上，在一个设定的时间主控程序将与大量代理程序通讯，代理程序已经被安装在网络上的许多计算机上。代理程序收到指令时就发动攻击。利用客户/服务器技术，主控程序能在几秒钟内激活成百上千次代理程序的运行。

##### 关于反射式DRDoS原理

DRDoS是英文“Distributed Reflection Denial of Service ”的缩写，中文意思是“分布式反射拒绝服务”。与DoS、DDoS不同，该方式靠的是发送大量带有被害者IP地址的数据包给攻击主机，然后攻击主机对IP地址源做出大量回应，形成拒绝服务攻击。

#### 二、攻击流程

##### DDoS攻击流程

要完成这个攻击流程，得至少需要三个步骤。

1. 攻击者手里必须控制大量肉鸡机器，并且分布式在互联互通分布在互联上。
2. 攻击者随时可以通过代理或者控制程序同时向所有肉鸡发送大量攻击指令。
3. 所有肉鸡在接受指令后，同时大量并发，同时向受害者网络或者主机发起攻击行为。

##### DRDoS攻击流程

DRDoS要完成一次反射放大攻击：

1. 攻击者，必须提前需要把攻击数据存放在所有的在线肉鸡或者反射服务器之上。
2. 攻击者，必须伪造IP源头。发送海量伪造IP来源的请求。当然这里的IP就是受害者的IP地址。
3. 反射服务器，必须可以反射数据，运行良好稳定。最好是请求数据少，返回数据成万倍增加。

如此不断循环，就可以大规模攻击其带宽网络，增加占用率和耗损目标机的硬件资源。

利用Memcached实现的DRDos攻击反射流程

![img](images/24d786f7-9aed-4b7e-af4f-dc60e9dd8641.png-w331s)

#### 三、存活机器

首先我们要找到大量反射服务器，利用搜索引擎去发掘全球可利用在线服务器。这里我暂时用zoomeye进行搜集，你也可以用别的搜索引擎，比如shodan等。默认开启端口号是11211,利用知道创宇得钟馗之眼空间引擎搜索到全球538317台机器开启11211端口，运行着Memcached缓存服务系统。但是利用条件还有一个，就是我们还得进一步筛选确认是否开启默认可以登录的机器，这样就可以被我们所利用了。有些已经设置了安全认证，那么就无法使用了。（这里就不公布了）

![img](images/b2b0d1ce-c50a-4c43-aeab-9adf5a8cdf4a.png-w331s)

#### 四、通信协议

从协议看，memcache同时监听tcp和udp。也就是说它本身支持两种协议同时可以发起交互和通信。这个就很关键了。大家可以看看tcp和udp协议区别。由于TCP是字节流，没有包的边界，无所谓大小，一次发送接受的数据取决于实现以及你的发送接收缓存大小。

TCP没有限定，TCP包头中就没有“包长度”字段，而完全依靠IP层去处理分帧。

![img](images/14ad4588-3505-4c94-b4b9-3c80fe9a77c2.png-w331s)

但是UDP协议就不一样了，他不基于连接，直接发送数据报到目标机器。

![img](images/d854080e-e2dc-40a0-8e56-380c2ae9b0e3.png-w331s)

注意这个Length字段，只占有两个字节。所以说UDP协议发送数据就有了限制，单次最大发送2^16=65535=64KB。

如果想要发送更大数据包，那么只能使用TCP协议或者UDP多次发送来实现，这里我已经测试过，两种协议均可以实现。

小结：

1. TCP面向连接（如打电话要先拨号建立连接）;UDP是无连接的，即发送数据之前不需要建立连接。
2. TCP提供可靠的服务。也就是说，通过TCP连接传送的数据，无差错，不丢失，不重复，且按序到达;UDP尽最大努力交 付，即不保证可靠交付。
3. TCP面向字节流，实际上是TCP把数据看成一连串无结构的字节流;UDP是面向报文的。UDP没有拥塞控制，因此网络出 现拥塞不会使源主机的发送速率降低（对实时应用很有用，如IP电话，实时视频会议等）。
4. 每一条TCP连接只能是点到点的;UDP支持一对一，一对多，多对一和多对多的交互通信。
5. TCP首部开销20字节;UDP的首部开销小，只有8个字节。
6. TCP的逻辑通信信道是全双工的可靠信道，UDP则是不可靠信道。

好了，明白了这个，我们就看看怎么利用基于TCP和UDP协议通信的Memcached缓存系统。由于Memcached系统支持最大键值单数据对1M存储。所以我们最大只能存储1M，当然你可以作多个字段，这样也会放大。那首先按照流程图我们向远程服务器提前存储有效载荷，这里就是数据了。利用TCP协议可以一次性发1M，但是我们要是利用UDP就得循环发送多次才能完成1M数据传输。由于UDP具有不稳定性，数据包不保证可靠交付。这里我建议使用TCP进行发送。

#### 五、数据格式

Memcached简洁而强大。它的简洁设计便于快速开发，减轻开发难度，解决了大数据量缓存的很多问题。它的API兼容大部分流行的开发语言。本质上，它是一个简洁的key-value存储系统。

一般的使用目的是，通过缓存数据库查询结果，减少数据库访问次数，以提高动态Web应用的速度、提高可扩展性。

![img](images/f503bc8d-74b9-4813-9a22-da719693f24c.png-w331s)

支持有如下所有命令和操作。

**Memcached 存储命令**
Memcached set 命令
Memcached add 命令
Memcached replace 命令
Memcached append 命令
Memcached prepend 命令
Memcached CAS 命令
**Memcached 查找命令**
Memcached get 命令
Memcached gets 命令
Memcached delete 命令
Memcached incr/decr 命令
**Memcached 统计命令**
Memcached stats 命令
Memcached stats items 命令
Memcached stats slabs 命令
Memcached stats sizes 命令
Memcached flush_all 命令

这里我们重点介绍三种命令，因为我们的攻击流程中将会涉及了这三种方式。

第一个是上传有效载荷Memcached set 命令

Memcached set 命令用于将 **value(数据值)** 存储在指定的 **key(键)** 中。

如果set的key已经存在，该命令可以更新该key所对应的原来的数据，也就是实现更新的作用。

set 命令的基本语法格式如下：

```
set key flags exptime bytes [noreply] 
value 
```

参数说明如下：

key：键值 key-value 结构中的 key，用于查找缓存值。
flags：可以包括键值对的整型参数，客户机使用它存储关于键值对的额外信息 。
exptime：在缓存中保存键值对的时间长度（以秒为单位，0 表示永远）
bytes：在缓存中存储的字节数
noreply（可选）： 该参数告知服务器不需要返回数据
value：存储的值（始终位于第二行）（可直接理解为key-value结构中的value）

![img](images/da37b102-db38-410f-86d5-c9de55c74c18.png-w331s)第二个反射有效载荷Memcached get 命令

Memcached get 命令获取存储在 **key(键)** 中的 **value(数据值)** ，如果 key 不存在，则返回空。

get 命令的基本语法格式如下：

```
get key
```

多个 key 使用空格隔开，如下:

```
get key1 key2 key3
```

参数说明如下：

key：键值 key-value 结构中的 key，用于查找缓存值。

![img](images/03aa4ad6-2ad4-4d38-b22f-a39729b05477.png-w331s)

第三个是退出远程服务器。quit\r\n命令即可，没有任何参数，注意要有回车换行。

![img](images/6e2a8af7-7e81-4fae-8457-248478d16962.png-w331s)

#### 六、攻击步骤

##### 自动化上传有效载荷

到了这里，我们接下来就是如何利用这个过程实现DRDoS反射拒绝服务攻击。

思路是这样的：我们先批量上传指定数据到远程开放服务器Memcached上面，然后我们再去Memcached服务器请求查询数据上一步存储的数据，（这一步很关键，我们只能利用UDP协议进行反射，后面说明一下为什么。）这样就可以将数据通过Memcached服务器反射到目标受害机器上了。这里我们可以自己手动编写程序实现批量自动化上传有效载荷到远程服务器，等待上传完了我们就可以进行UDP反射攻击了。

这里我用python脚本完成payload数据上传。

代码就不公布了，防止非法利用。直接输出测试结果。

输出结果

![img](images/d22af630-1ff1-4ed2-ab42-15c3bfaeba4f.png-w331s)

##### 自动化反射有效载荷

这里得注意一下，上面的自动化上传我使用了TCP协议发送数据包，反射我必须使用UDP协议。因为只有UDP协议是基于无连接的，这样我们直接发送数据到目标服务器，不需要进行三次握手。同时服务器接收方也无法验证客户源IP，因此这个过程我们才可以利用UDP伪造源地址，实现反射DRDoS攻击过程。

利用socket和scapy库开发，采用多线程进行循环请求。（特别注意UDP协议使用的时候，每个操作命令必须都要添加数据包结构要加头部8字节标志位， "\x00\x00\x00\x00\x00\x01\x00\x00"）

这里使用python脚本完成反射攻击。

代码就不公布了，防止非法利用。直接输出测试结果。

输出，可以实现

![img](images/555b4b58-c98e-4da9-b852-40065c2816f5.png-w331s)

测试wireshark抓包

![img](images/c0238011-7df3-4ac5-901c-e9d91a79a197.png-w331s)

这里不妨可以进行一个大概理论计算

比如单台服务器我们虽然只发送的攻击指令只有二十个字节数据，但却可以返回1M数据。1M/20=5W（5万倍放大率），这可谓四两拨千斤。假设理想状况下我们现在手里有50W可用机器，那么我们的DRDoS理论值数值将会达到约50W*1M=500GB。大家想想这个是多么恐怖的带宽和数据。现在目前国内能够抵御短时间发起这么大的DDoS攻击的，几乎没有。比如去年攻击阿里云超过14小时,峰值流量达到453.8G。而DRDos可以只需要一分钟就能实现高达500G流量，这将是多么可怕的事情，多大的灾难。

#### 七、总结体会

关于这项DRDoS技术经过几天摸索研究也算已经了解清楚了，但是测试中发现有的网络环境里面会被一些路由器纠正源地址，使得反射攻击失败。究其原因是因为其增加的uRPF机制，（Unicast Reverse Path Forwarding是一种单播反向路由查找技术，用于防止基于源地址欺骗的网络攻击行为。）重新修复了UDP源地址伪造。不过有些环境中没有这种机制的，那么我们就可以利用此方法实现攻击。在这里分享给大家，希望可以有人继续深入分析和钻研，其中涉及利用的思路和技巧也可学习学习。比如说利用其免费的互联网存储资源，将你的数据源进行分布式存储，当做你的分布式私密云盘。

友情提醒：以上仅做技术研究，如有恶意利用，发起攻击，将自负法律后果

#### 八、参考学习

[http://memcached.org](http://memcached.org/)

http://www.runoob.com/memcached/memcached-tutorial.html

https://baike.baidu.com/item/DRDoS/9222989?fr=aladdin

http://artur.ejsmont.org/blog/content/memcache-over-udp-transport-protocol-is-not-really-so-cool

https://github.com/memcached/memcached/blob/master/doc/protocol.txt

[https://baike.baidu.com/item/%E5%88%86%E5%B8%83%E5%BC%8F%E6%8B%92%E7%BB%9D%E6%9C%8D %E5%8A%A1%E6%94%BB%E5%87%BB/3802159?fr=aladdin&fromid=444572&fromtitle=DDOS](https://baike.baidu.com/item/分布式拒绝服 务攻击/3802159?fr=aladdin&fromid=444572&fromtitle=DDOS)

