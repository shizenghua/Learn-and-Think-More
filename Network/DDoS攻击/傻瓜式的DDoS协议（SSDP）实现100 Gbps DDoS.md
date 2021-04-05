# 傻瓜式的DDoS协议（SSDP）实现100 Gbps DDoS

src:https://zhuanlan.zhihu.com/p/27680875



上个月我们统计了一些ddos攻击数据。平均的SSDP攻击大小是1〜12 Gbps，我们记录的最大的SSDP反射是：

- 30 Mpps（每秒数百万个数据包）
- 80 Gbps（每秒数十亿位）
- 使用940k反射IP

几天前，当我们注意到非常大的SSDP放大时，这改变了。这是值得深入调查的，因为它超过了100 Gbps的阈值。

攻击期间每秒数据包的数据如下图所示：

带宽使用情况：

这个包洪水持续了38分钟。根据我们采样的netflow数据，它使用了930k反射服务器。我们估计，在38分钟内，每个服务器发送112k包到Cloudflare。

反射器服务器遍布全球，在阿根廷，俄罗斯和中国都有部署。以下是每个国家/地区的唯一IP：

```text
$ cat ips-nf-ct.txt|uniq|cut -f 2|sort|uniq -c|sort -nr|head
 439126 CN
 135783 RU
  74825 AR
  51222 US
  41353 TW
  32850 CA
  19558 MY
  18962 CO
  14234 BR
  10824 KR
  10334 UA
   9103 IT
   ...
```

ASN上的反射器IP分布是常见的。它几乎符合世界上最大的住宅ISPs：

```text
$ cat ips-nf-asn.txt |uniq|cut -f 2|sort|uniq -c|sort -nr|head
 318405 4837   # CN China Unicom
  84781 4134   # CN China Telecom
  72301 22927  # AR Telefonica de Argentina
  23823 3462   # TW Chunghwa Telecom
  19518 6327   # CA Shaw Communications Inc.
  19464 4788   # MY TM Net
  18809 3816   # CO Colombia Telecomunicaciones
  11328 28573  # BR Claro SA
   7070 10796  # US Time Warner Cable Internet
   6840 8402   # RU OJSC "Vimpelcom"
   6604 3269   # IT Telecom Italia
   6377 12768  # RU JSC "ER-Telecom Holding"
   ...
```

## 什么是SSDP？

攻击由源端口1900的UDP数据包组成。该端口由[SSDP](https://link.zhihu.com/?target=https%3A//en.wikipedia.org/wiki/Simple_Service_Discovery_Protocol)使用，由UPnP协议使用。UPnP是[零配置网络](https://link.zhihu.com/?target=https%3A//en.wikipedia.org/wiki/Zero-configuration_networking%23UPnP)协议之一。您的家庭设备很可能支持它，当新设备（如笔记本电脑）加入网络时，允许您的电脑或手机轻松的发现他们。当新设备加入时，可以向本地网络查询特定设备，如互联网网关，音频系统，电视机或打印机。



[UPnP](https://link.zhihu.com/?target=http%3A//www.upnp-hacks.org/upnp.html)标准很糟糕，但这里是关于框架[规范](https://link.zhihu.com/?target=https%3A//web.archive.org/web/20151107123618/http%3A//upnp.org/specs/arch/UPnP-arch-DeviceArchitecture-v2.0.pdf)的片段M-SEARCH- 主要的发方法：

> *当控制点被添加到网络时，UPnP发现协议允许控制点在网络上搜索感兴趣的设备。它通过在保留的地址和端口（239.255.255.250:1900）上广播具有设备或服务的类型或标识符的数据或目标的搜索消息来实现。*

响应M-SEARCH框架：

> *要通过网络搜索找到设备，设备应向发送请求的源IP地址和端口向组播地址发送UDP响应。如果M-SEARCH请求的ST头字段是“ssdp：all”，“upnp：rootdevice”，“uuid：”，然后是与设备发布的UUID完全相同的UUID，或者如果M-SEARCH请求与设备类型匹配设备支持的服务类型。*

这在实践中很有意思。例如，我猜我的Chrome浏览器经常向一个智能电视发送请求：



```text
$ sudo tcpdump -ni eth0 udp and port 1900 -A
IP 192.168.1.124.53044 > 239.255.255.250.1900: UDP, length 175  
M-SEARCH * HTTP/1.1  
HOST: 239.255.255.250:1900  
MAN: "ssdp:discover"  
MX: 1  
ST: urn:dial-multiscreen-org:service:dial:1  
USER-AGENT: Google Chrome/58.0.3029.110 Windows  
```

该帧发送到组播IP地址。监听该地址并支持这种特定ST（搜索目标）多屏幕类型的其他设备应该回答。

除了对特定设备类型的查询外，还有两种“通用” ST查询类型：

- upnp:rootdevice：搜索根设备
- ssdp:all：搜索所有UPnP设备和服务

要模拟这些查询，您可以运行此python脚本（基于[此项目](https://zhuanlan.zhihu.com/)）：

```python
#!/usr/bin/env python2
import socket  
import sys

dst = "239.255.255.250"  
if len(sys.argv) > 1:  
    dst = sys.argv[1]
st = "upnp:rootdevice"  
if len(sys.argv) > 2:  
    st = sys.argv[2]

msg = [  
    'M-SEARCH * HTTP/1.1',
    'Host:239.255.255.250:1900',
    'ST:%s' % (st,),
    'Man:"ssdp:discover"',
    'MX:1',
    '']

s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)  
s.settimeout(10)  
s.sendto('\r\n'.join(msg), (dst, 1900) )

while True:  
    try:
        data, addr = s.recvfrom(32*1024)
    except socket.timeout:
        break
    print "[+] %s\n%s" % (addr, data)
```

在我的家庭网络上，显示两个设备：

```text
$ python ssdp-query.py
[+] ('192.168.1.71', 1026)
HTTP/1.1 200 OK  
CACHE-CONTROL: max-age = 60  
EXT:  
LOCATION: http://192.168.1.71:5200/Printer.xml  
SERVER: Network Printer Server UPnP/1.0 OS 1.29.00.44 06-17-2009  
ST: upnp:rootdevice  
USN: uuid:Samsung-Printer-1_0-mrgutenberg::upnp:rootdevice

[+] ('192.168.1.70', 36319)
HTTP/1.1 200 OK  
Location: http://192.168.1.70:49154/MediaRenderer/desc.xml  
Cache-Control: max-age=1800  
Content-Length: 0  
Server: Linux/3.2 UPnP/1.0 Network_Module/1.0 (RX-S601D)  
EXT:  
ST: upnp:rootdevice  
USN: uuid:9ab0c000-f668-11de-9976-000adedd7411::upnp:rootdevice  
```

## 防火墙

现在我们了解SSDP的基础知识，理解反射攻击应该很简单。你看，实际上有两种交换方式M-SEARCH：

- 我们提出了多播地址
- 直接通过普通单播地址启用UPnP / SSDP主机

后一种更方法有效。我们可以专门针对我的打印机IP地址：

```text
$ python ssdp-query.py 192.168.1.71
[+] ('192.168.1.71', 1026)
HTTP/1.1 200 OK  
CACHE-CONTROL: max-age = 60  
EXT:  
LOCATION: http://192.168.1.71:5200/Printer.xml  
SERVER: Network Printer Server UPnP/1.0 OS 1.29.00.44 06-17-2009  
ST: upnp:rootdevice  
USN: uuid:Samsung-Printer-1_0-mrgutenberg::upnp:rootdevice  
```

现在很容易看到问题：SSDP协议不检查查询方是否与设备在同一个网络中。它将响应M-SEARCH通过公共互联网传递的信息。所有这一切都是防火墙中的一个微小的错误配置 - 端口1900 UDP对外部开放，并且UDP扩展的目标将可利用。



由于配置错误的目标，我们的脚本将在互联网上工作：

```text
$ python ssdp-query.py 100.42.x.x
[+] ('100.42.x.x', 1900)
HTTP/1.1 200 OK  
CACHE-CONTROL: max-age=120  
ST: upnp:rootdevice  
USN: uuid:3e55ade9-c344-4baa-841b-826bda77dcb2::upnp:rootdevice  
EXT:  
SERVER: TBS/R2 UPnP/1.0 MiniUPnPd/1.2  
LOCATION: http://192.168.2.1:40464/rootDesc.xml  
```

## 放大

真正的攻击是由ssdp:all ST类型来完成的。这些效果*要*大得多：

```text
$ python ssdp-query.py 100.42.x.x ssdp:all
[+] ('100.42.x.x', 1900)
HTTP/1.1 200 OK  
CACHE-CONTROL: max-age=120  
ST: upnp:rootdevice  
USN: uuid:3e55ade9-c344-4baa-841b-826bda77dcb2::upnp:rootdevice  
EXT:  
SERVER: TBS/R2 UPnP/1.0 MiniUPnPd/1.2  
LOCATION: http://192.168.2.1:40464/rootDesc.xml

[+] ('100.42.x.x', 1900)
HTTP/1.1 200 OK  
CACHE-CONTROL: max-age=120  
ST: urn:schemas-upnp-org:device:InternetGatewayDevice:1  
USN: uuid:3e55ade9-c344-4baa-841b-826bda77dcb2::urn:schemas-upnp-org:device:InternetGatewayDevice:1  
EXT:  
SERVER: TBS/R2 UPnP/1.0 MiniUPnPd/1.2  
LOCATION: http://192.168.2.1:40464/rootDesc.xml

... 6 more response packets....
```

在这种特殊情况下，单个SSDP M-SEARCH报文发送了8个响应报文。tcpdump视图：

```text
$ sudo tcpdump -ni en7 host 100.42.x.x -ttttt
 00:00:00.000000 IP 192.168.1.200.61794 > 100.42.x.x.1900: UDP, length 88
 00:00:00.197481 IP 100.42.x.x.1900 > 192.168.1.200.61794: UDP, length 227
 00:00:00.199634 IP 100.42.x.x.1900 > 192.168.1.200.61794: UDP, length 299
 00:00:00.202938 IP 100.42.x.x.1900 > 192.168.1.200.61794: UDP, length 295
 00:00:00.208425 IP 100.42.x.x.1900 > 192.168.1.200.61794: UDP, length 275
 00:00:00.209496 IP 100.42.x.x.1900 > 192.168.1.200.61794: UDP, length 307
 00:00:00.212795 IP 100.42.x.x.1900 > 192.168.1.200.61794: UDP, length 289
 00:00:00.215522 IP 100.42.x.x.1900 > 192.168.1.200.61794: UDP, length 291
 00:00:00.219190 IP 100.42.x.x.1900 > 192.168.1.200.61794: UDP, length 291
```

该目标公开了8x数据包计数放大和26倍带宽放大。可悲的是，这是SSDP的日常。

## IP欺骗

攻击的最后一步是欺骗易受攻击的服务器，以淹没目标IP，而不是攻击者。为此攻击者需要[欺骗](https://link.zhihu.com/?target=https%3A//en.wikipedia.org/wiki/IP_address_spoofing)他们的查询[源IP地址](https://link.zhihu.com/?target=https%3A//en.wikipedia.org/wiki/IP_address_spoofing)。

我们探测了所示的100 Gbps +攻击中使用的反射器IP。我们发现，在920k反射器IP中，只有350k（38％）仍然响应SSDP探测器。

在响应的反射器中，每个发送平均7个数据包：

```text
$ cat results-first-run.txt|cut -f 1|sort|uniq -c|sed -s 's#^ \+##g'|cut -d " " -f 1| ~/mmhistogram -t "Response packets per IP" -p
Response packets per IP min:1.00 avg:6.99 med=8.00 max:186.00 dev:4.44 count:350337  
Response packets per IP:  
 value |-------------------------------------------------- count
     0 |                    ****************************** 23.29%
     1 |                                              ****  3.30%
     2 |                                                **  2.29%
     4 |************************************************** 38.73%
     8 |            ************************************** 29.51%
    16 |                                               ***  2.88%
    32 |                                                    0.01%
    64 |                                                    0.00%
   128 |                                                    0.00%
```

响应数据包平均有321字节（+/- 29字节）。我们的请求包有110个字节。

根据我们与ssdp:all M-SEARCH攻击者的测量可以实现：

- 7x包放大
- 20倍带宽放大

我们可以估计43 Mpps / 112 Gbps攻击是大致生成的：

- 6.1Mpps的欺骗能力
- 5.6 Gbps的欺骗带宽

换句话说：能够执行IP欺骗的单一连接好的10Gbps服务器可以提供强大的SSDP攻击。

## 更多SSDP服务器

由于我们探测到易受攻击的SSDP服务器，所以这里是Server我们收到的最常见的响应头：

```text
 104833 Linux/2.4.22-1.2115.nptl UPnP/1.0 miniupnpd/1.0
  77329 System/1.0 UPnP/1.0 IGD/1.0
  66639 TBS/R2 UPnP/1.0 MiniUPnPd/1.2
  12863 Ubuntu/7.10 UPnP/1.0 miniupnpd/1.0
  11544 ASUSTeK UPnP/1.0 MiniUPnPd/1.4
  10827 miniupnpd/1.0 UPnP/1.0
   8070 Linux UPnP/1.0 Huawei-ATP-IGD
   7941 TBS/R2 UPnP/1.0 MiniUPnPd/1.4
   7546 Net-OS 5.xx UPnP/1.0
   6043 LINUX-2.6 UPnP/1.0 MiniUPnPd/1.5
   5482 Ubuntu/lucid UPnP/1.0 MiniUPnPd/1.4
   4720 AirTies/ASP 1.0 UPnP/1.0 miniupnpd/1.0
   4667 Linux/2.6.30.9, UPnP/1.0, Portable SDK for UPnP devices/1.6.6
   3334 Fedora/10 UPnP/1.0 MiniUPnPd/1.4
   2814  1.0
   2044 miniupnpd/1.5 UPnP/1.0
   1330 1
   1325 Linux/2.6.21.5, UPnP/1.0, Portable SDK for UPnP devices/1.6.6
    843 Allegro-Software-RomUpnp/4.07 UPnP/1.0 IGD/1.00
    776 Upnp/1.0 UPnP/1.0 IGD/1.00
    675 Unspecified, UPnP/1.0, Unspecified
    648 WNR2000v5 UPnP/1.0 miniupnpd/1.0
    562 MIPS LINUX/2.4 UPnP/1.0 miniupnpd/1.0
    518 Fedora/8 UPnP/1.0 miniupnpd/1.0
    372 Tenda UPnP/1.0 miniupnpd/1.0
    346 Ubuntu/10.10 UPnP/1.0 miniupnpd/1.0
    330 MF60/1.0 UPnP/1.0 miniupnpd/1.0
    ...
```

ST我们看到的最常见的头值：

```text
 298497 upnp:rootdevice
 158442 urn:schemas-upnp-org:device:InternetGatewayDevice:1
 151642 urn:schemas-upnp-org:device:WANDevice:1
 148593 urn:schemas-upnp-org:device:WANConnectionDevice:1
 147461 urn:schemas-upnp-org:service:WANCommonInterfaceConfig:1
 146970 urn:schemas-upnp-org:service:WANIPConnection:1
 145602 urn:schemas-upnp-org:service:Layer3Forwarding:1
 113453 urn:schemas-upnp-org:service:WANPPPConnection:1
 100961 urn:schemas-upnp-org:device:InternetGatewayDevice:
 100180 urn:schemas-upnp-org:device:WANDevice:
  99017 urn:schemas-upnp-org:service:WANCommonInterfaceConfig:
  98112 urn:schemas-upnp-org:device:WANConnectionDevice:
  97246 urn:schemas-upnp-org:service:WANPPPConnection:
  96259 urn:schemas-upnp-org:service:WANIPConnection:
  93987 urn:schemas-upnp-org:service:Layer3Forwarding:
  91108 urn:schemas-wifialliance-org:device:WFADevice:
  90818 urn:schemas-wifialliance-org:service:WFAWLANConfig:
  35511 uuid:IGD{8c80f73f-4ba0-45fa-835d-042505d052be}000000000000
   9822 urn:schemas-upnp-org:service:WANEthernetLinkConfig:1
   7737 uuid:WAN{84807575-251b-4c02-954b-e8e2ba7216a9}000000000000
   6063 urn:schemas-microsoft-com:service:OSInfo:1
    ...
```

易受攻击的IP似乎大多是无保护的家庭路由器。

## 打开SSDP是一个漏洞

允许UDP端口1900从互联网到家庭打印机的流量并不是一个好主意。自2013年1月至今，已经有这个问题：

[“通用即插即用的安全漏洞：拔下，不用”](https://link.zhihu.com/?target=https%3A//community.rapid7.com/community/infosec/blog/2013/01/29/security-flaws-in-universal-plug-and-play-unplug-dont-play)

SSDP的作者显然没有考虑UDP扩增潜力。关于未来使用SSDP协议有一些明显的建议：

- 如果真实世界使用单播M-SEARCH查询，SSDP的作者应该回答。从我所理解的M-SEARCH只是在局域网中作为组播查询的实际意义。
- 单播M-SEARCH支持应该被弃用或至少是限制速率，类似于[DNS响应速率限制技术](https://link.zhihu.com/?target=http%3A//www.redbarn.org/dns/ratelimits)。
- M-SEARCH响应应该只传送到本地网络。通过网络路由的响应几乎没有意义和开放的描述漏洞。

同时我们建议：

- 网络管理员应确保接入UDP端口：1900被防火墙阻止。
- 互联网服务供应商应该不允许在其网络上执行IP欺骗。IP欺骗是问题的根本原因。比如臭名昭着的[BCP38](https://link.zhihu.com/?target=http%3A//www.bcp38.info/index.php/Main_Page)。
- 互联网服务提供商应允许他们的客户使用BGP流量来限制入站UDP源端口1900流量，以减轻大型SSDP攻击期间的拥塞。
- 互联网提供商应在内部收集网络流量协议样本。需要netflow来识别攻击的真正来源。使用netflow来回答以下问题是微不足道的：“我的哪个客户向网关发送了6.4Mbps的流量？” 由于隐私问题，我们建议收集最大可能采样值的netflow样本：64K数据包中的1。这将足以跟踪DDoS攻击，同时保留单个客户连接的体面隐私。
- 开发人员不要在不仔细考虑UDP扩展问题的情况下推出自己的UDP协议。UPnP应适当标准化和审查。
- 鼓励最终用户使用脚本扫描其网络以启用UPnP设备。考虑是否允许这些设备访问互联网。

此外，我们准备在线检查网站。如果您想知道您的公共IP地址是否具有易受攻击的SSDP服务，请单击：

[Bad UPnP/SSDP - Check for WAN UPnP listening](https://link.zhihu.com/?target=https%3A//badupnp.benjojo.co.uk/)

令人遗憾的是，我们在描述的攻击中看到的最无保护的路由器来自中国，俄罗斯和阿根廷，这些地方对于最敏捷的互联网服务提供商而言并不知名。

## 概要

Cloudflare客户完全免受SSDP和其他L3放大攻击。这些攻击很好地被[Cloudflare anycast](https://link.zhihu.com/?target=https%3A//blog.cloudflare.com/how-cloudflares-architecture-allows-us-to-scale-to-stop-the-largest-attacks/)基础设施所偏转，不需要特别的操作。不幸的是，提高SSDP攻击大小可能是其他互联网公民的难题。我们应该鼓励我们的ISP在其网络内停止IP欺骗，支持BGP流规范，并配置在netflow中。