# DDos/DoS工具集
src:https://blog.csdn.net/wuzhimang/article/details/54666788?utm_medium=distribute.pc_relevant_download.none-task-blog-blogcommendfrombaidu-14.nonecase&depth_1-utm_source=distribute.pc_relevant_download.none-task-blog-blogcommendfrombaidu-14.nonecas



>项目中，需要帮助某公司完成对几台抗DDoS设备的测试，受限于各类因素，只能通过搭建软件环境来完成测评。下面是针对自己前期工作的一小部分整理，仅罗列了基于开源工具的一些内容，其他定制化的和自己编写的程序则未公开。hping、LOIC等其他工具，由于性能和其它原因也未被使用！
[github ddos-dos-tools](https://github.com/wenfengshi/ddos-dos-tools)

# 1. SynFlood 攻击
>借助`netsniff-ng`套件中的`trafgen`工具，其可伪造源ip发起DDoS攻击

* `trafgen`是一款高速的，多线程数据包生成器，官方测试显示其速度可达到12Mpps，自己在`Intel(R) Xeon(R) CPU E5-2620 v3 @ 2.40GHz`下测得的发包速率有500Mbit/s多。通过对比其他开源程序，本工具的发包性能是自己测试中性能表现最高的。

* [synflood.trafgen](https://github.com/wenfengshi/ddos-dos-tools/blob/master/synflood.trafgen)是对应的配置文件模版，修改文件里的源／目的MAC地址以及源／目的IP后，命令行直接运行`trafgen --cpp --dev eth0 --conf ackflood.trafgen --cpu 2 --verbose`即可发起synflood攻击

* 通过添加`trafgen`命令行参数`--gap`修改发包的速率，具体请`man trafgen`

* 对应工具可直接通过在线源进行安装，CentOS下`yum install netsniff-ng`即可安装整个套件，其中包含trafgen等工具。（预先可能需安装fedora源，`yum install epel-release.noarch -y`）

# 2. AckFlood 攻击
> 同SynFlood类似

* [ackflood.trafgen](https://github.com/wenfengshi/ddos-dos-tools/blob/master/ackflood.trafgen)是对应的配置文件模版，修改文件里的源／目的MAC地址以及源／目的IP后，命令行直接运行`trafgen --cpp --dev eth0 --conf ackflood.trafgen --cpu 2 --verbose`即可发起ackflood攻击

# 3. SSL 攻击
[thc-ssl-dos](https://github.com/wenfengshi/ddos-dos-tools/tree/master/thc-ssl-dos)是一款有名的ssl攻击程序，原理是ssl重新协商机制，但对于关闭了的或不支持SSL重协商的服务端，该工具将失效。
[ssl-dos.sh](https://github.com/wenfengshi/ddos-dos-tools/blob/master/ssl-dos.sh)是自己写的一个简单的ssl攻击脚本，且适用于不支持ssl重协商的服务端，该脚本借助的是openssl工具。

# 4. HTTP GET 攻击
* [http-get-dos](https://github.com/wenfengshi/ddos-dos-tools/tree/master/http-get-dos)是一个简单的、高性能HTTP GET DOS工具，可自定义HTTP请求头、连接数、总的HTTP请求数等
* 进入目录下`make`编译后，`http-get-dos -h` 查看使用信息

# 5. HTTP 慢速攻击
[pyloris](https://github.com/wenfengshi/ddos-dos-tools/tree/master/pyloris)是一款开源的HTTP慢速DOS攻击软件，本版本为3.2版本，详情见[主页](https://motoma.io/pyloris/)，含图形界面，使用很方便

# 6. UDP fragment 攻击
> 同SynFlood类似

[small_frag.trafgen](https://github.com/wenfengshi/ddos-dos-tools/blob/master/small_frag.trafgen)是对应的配置文件模版，修改文件里的源／目的MAC地址以及源／目的IP后，命令行直接运行`trafgen --cpp --dev eth0 --conf small_frag.trafgen --cpu 2 --verbose`即可发起UDP fragment DoS attack攻击

