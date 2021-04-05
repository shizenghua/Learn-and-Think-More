# 通过shell脚本防止端口扫描

src:https://blog.csdn.net/weixin_33672400/article/details/92578330



网上有现在的防端口工具如psad、portsentry但觉得配置有点麻烦且服务器不想再装一个额外的软件。所以可以自己写个shell脚本实现这个功能。基本思路是使用iptables的recent模块记录下在60秒钟内扫描超过10个端口的IP并结合inotify-tools工具实时监控iptables的日志一旦iptables日志文件有写入新的ip记录则使用iptables封锁源ip起到了防止端口扫描的功能。

# 1、iptables规则设置
新建脚本iptables.sh执行此脚本。

```bash
IPT="/sbin/iptables"
   $IPT --delete-chain
   $IPT --flush

   #Default Policy
   $IPT -P INPUT DROP  
   $IPT -P FORWARD DROP
   $IPT -P OUTPUT DROP

   #INPUT Chain
   $IPT -A INPUT -m state --state RELATED,ESTABLISHED -j ACCEPT
   $IPT -A INPUT -p tcp -m tcp --dport 80 -j ACCEPT
   $IPT -A INPUT -p tcp -m tcp --dport 22 -j ACCEPT
   $IPT -A INPUT -i lo -j ACCEPT
   $IPT -A INPUT -p icmp -m icmp --icmp-type 8 -j ACCEPT
   $IPT -A INPUT -p icmp -m icmp --icmp-type 11 -j ACCEPT
   $IPT -A INPUT -p tcp --syn -m recent --name portscan --rcheck --seconds 60 --hitcount 10 -j LOG
   $IPT -A INPUT -p tcp --syn -m recent --name portscan --set -j DROP
   #OUTPUT Chain
   $IPT -A OUTPUT -m state --state RELATED,ESTABLISHED -j ACCEPT
   $IPT -A OUTPUT -p udp -m udp --dport 53 -j ACCEPT
   $IPT -A OUTPUT -o lo -j ACCEPT
   $IPT -A OUTPUT -p icmp -m icmp --icmp-type 8 -j ACCEPT
   $IPT -A OUTPUT -p icmp -m icmp --icmp-type 11 -j ACCEPT

   #iptables save
   service iptables save
   service iptables restart
```

注意17-18行的两条规则务必在INPUT链的最下面其它规则自己可以补充。

# 2、iptables日志位置更改

编辑`/etc/syslog.conf`

添加`kern.warning /var/log/iptables.log`

重启syslog
`/etc/init.d/syslog restart`

# 3、防端口扫描shell脚本

首先安装inotify:

`yum install inotify-tools`

保存以下代码为ban-portscan.sh

```bash
   btime=600 #封ip的时间
   while true;do
       while inotifywait -q -q -e modify /var/log/iptables.log;do
           ip=`tail -1 /var/log/iptables.log | awk -F"[ =]" '{print $13}' | grep '[0−9]{1,3}\.\{3\}[0-9]\{1,3\}'`
           if test -z "`/sbin/iptables -nL | grep $ip`";then
               /sbin/iptables -I INPUT -s $ip -j DROP
               {
               sleep $btime && /sbin/iptables -D INPUT -s $ip -j DROP
               } &
           fi
       done
   done
```

执行命令开始启用端口防扫描

`nohup ./ban-portscan.sh &`