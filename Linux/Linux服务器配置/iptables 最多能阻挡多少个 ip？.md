# iptables 最多能阻挡多少个 ip？

2017-05-16 09:21:12 +08:00

![unboy](images/a8f0f72de95d8e835eb3399ddf41eeb6)

 unboy

centos6.5 x64 系统
近期服务器受到大量 ip 攻击，抓包筛选出大约 10 万个 ip,拟导入 iptables 拦截，请问 iptables 是否支持这么多的 ip,负载和性能上会有多大影响？此前有无相关数据测试？

4606 次点击

所在节点 

![Linux](images/11_large.png)

 [Linux](https://www.v2ex.com/go/linux)



21 条回复

![tony1016](images/01424f111450a1d74377af2195254115)

tony1016

2017-05-16 09:24:00 +08:00

科学上网经验告诉我们，这么大的列表还是配合 ipset 用最快

![aksoft](images/2c62497aaa5c065e3a012aa82dc111e5)

aksoft

2017-05-16 09:30:06 +08:00

@[tony1016](https://www.v2ex.com/member/tony1016) +1 学习了

![qianguozheng](images/8b3f6f3484bca4c76c2f536c6837cff6)

qianguozheng

2017-05-16 09:34:18 +08:00

ipset 正解，首先你要了解的 netfilter 的原理，说白了整体逻辑就是遍历每一个 HOOK 上注册的表，表上建立的链， 如果觉得复杂，就理解为链表，每一个都遍历，数量越多，一个数据包走过的路程就越多，相对来说就越慢。

如果你的规则有共通性，当然可以通过 ipset 的 hash 方式来处理，哈希表查找起来肯定比链表快。

![unboy](images/a8f0f72de95d8e835eb3399ddf41eeb6)

unboy

2017-05-16 09:37:10 +08:00

@[tony1016](https://www.v2ex.com/member/tony1016)
@[qianguozheng](https://www.v2ex.com/member/qianguozheng) 原来如此，学习了。感谢。

![psfang](images/221451_large.png)

psfang

2017-05-16 09:38:07 +08:00

之前遇到过，加的 IP 太多，性能消耗太大，跪了。。

![QQ2171775959](images/228593_large.png)

QQ2171775959

2017-05-16 10:13:08 +08:00

这个只能做 IP 流量屏蔽了，不过这个只能简单的处理，如果别继续用新的 IP 来攻击，还是会有一样的问题出现，最好是做个 IP 防御策略，对不正当的流量进行 CDN 清洗。

![Jodal](images/203639_large.png)

Jodal

2017-05-16 11:04:59 +08:00

ipset 加进 hash，iptables drop 掉，复杂度是 O(1)。

![Showfom](images/5408_large.png)

Showfom

2017-05-16 12:48:47 +08:00

这么多 ip 不适合在服务器端弄了 买硬件防火墙吧

![rrfeng](images/21425_large.png)

rrfeng

2017-05-16 12:56:00 +08:00

只有我觉得 100000 IP 不太可能吗？这么大规模的攻击你确定有待遇享受？

还是再分析一下来源好

![zhs227](images/56156_large.png)

zhs227

2017-05-16 12:56:48 +08:00

用 ipset。但是 100,000 有点多了，感觉不是真实来源 IP。

![gdtv](images/a97045e4810bd1a809de8aea971b71c8)

gdtv

2017-05-16 13:25:22 +08:00

楼上说 10 万 ip 不可能，我前不久就遇到了类似的情况，有几万个 ip，，后来用 ipset 阻挡了

![unboy](images/a8f0f72de95d8e835eb3399ddf41eeb6)

unboy

2017-05-16 13:25:31 +08:00

@[rrfeng](https://www.v2ex.com/member/rrfeng)
@[zhs227](https://www.v2ex.com/member/zhs227) 差不多是这么多，这些 ip 都是慢速攻击，过好几秒才发送一次请求，每次请求的字节也很小，但是海量 ip 过来把带宽堵满了。

![rrfeng](images/21425_large.png)

rrfeng

2017-05-16 14:11:41 +08:00

什么服务？请求是什么样的？反射攻击吗？还是网页挂马之类的？
10 万即使封了，不会影响正常用户吗？

![gstqc](images/165783_large.png)

gstqc

2017-05-16 14:17:19 +08:00

\1. 抓包的不一定准确，因为数据包可能是伪造来源 IP。最好确定是否是真实 IP
\2. 10 万 IP 很正常，我的 IP 信誉库里有百万以上的黑名单。可以用工具将 IP 进行合并，很可能有些是整个 C 段都是攻击来源，合并后方便管理。

![unboy](images/a8f0f72de95d8e835eb3399ddf41eeb6)

unboy

2017-05-16 14:23:27 +08:00

@[rrfeng](https://www.v2ex.com/member/rrfeng) 还没封，这不才在探讨这个问题。

@[gstqc](https://www.v2ex.com/member/gstqc) 请推荐个好用的 ip 自动合并工具。

![gstqc](images/165783_large.png)

gstqc

2017-05-16 14:33:17 +08:00

@[unboy](https://www.v2ex.com/member/unboy) 可以写程序来处理，没有现成的工具。
我们一般是查 IP 来源，结合多个 IP 数据库来识别。
Python 推荐 ipaddress 这个库，能想到的计算方式都支持。

初期可以简单的用 IP 列表，数量不会影响到多少效率。
合并只是方便后续维护和更新。

![Jodal](images/203639_large.png)

Jodal

2017-05-16 17:27:20 +08:00

\1. 抓包确实不太准确，数据包是可以伪造，也可能是肉鸡。如果大量 block 的话，会影响相应的服务，并且现在客户端都是 Nat，block 掉之后，有可能是一大批用户没法使用。PS: netstat 可以输出这些 IP

\2. 尽量要隐藏服务端的源 IP，比如 cloudflare 服务，但是国内好像做的都不是很好。
\3. 买 DDos 防火墙，但是源 IP 暴露的话，也很难办。
\4. ipset 性能其实很好，10w IP 的话，机器应该很容易扛过去。
\5. 需要查看 ipset 的话，直接 man ipset
\6. 以下是相关的测评

https://workshop.netfilter.org/2013/wiki/images/a/ab/Jozsef_Kadlecsik_ipset-osd-public.pdf
https://strongarm.io/blog/linux-firewall-performance-testing/

![ericFork](images/29383_large.png)

ericFork

2017-05-16 18:48:07 +08:00

看到这里突发奇想，有人知道 Windows Server 高级防火墙的按 IP 阻挡的实现是链表还是哈希么？

![xierch](images/99f51f0bed22226f7d9d2c91606495a1)

xierch

2017-05-16 23:23:17 +08:00

顺带一提，除了用 iptables + ipset，还能用 nftables
https://wiki.nftables.org/wiki-nftables/index.php/Sets

![kiss2013star](images/8f7bcb9e458eefc4a0fd68aafcea783c)

kiss2013star

2017-05-17 10:18:37 +08:00

@[zhs227](https://www.v2ex.com/member/zhs227)