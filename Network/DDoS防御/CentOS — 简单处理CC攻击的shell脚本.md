# CentOS — 简单处理CC攻击的shell脚本

src:https://www.cnblogs.com/bruceleeliya/archive/2012/11/10/2763858.html



第一个脚本是通过查找日志中访问次数过多的ip,并用iptables屏蔽。

```bash
#!/bin/bash
cur=`date +%H%M%S`
becur=`date -d "1 minute ago" +%H%M%S`
badip=`tail -n 10000 /home/www.centos.bz/log/access.log | egrep -v "\.(gif|jpg|jpeg|png|css|js)" | awk  -v a="$becur" -v b="$cur" -F [' ':] '{t=$5$6$7;if (t>=a && t<=b) print $1}' | sort | uniq -c | awk '{if ($1>=20) print $2}'`
if [ ! -z "$badip" ];then
for ip in $badip;
do
if test -z "`/sbin/iptables -nL | grep $ip`";then
/sbin/iptables -I INPUT -s $ip -j DROP
fi
done
fi
```

将此代码保存为ban.sh，加入cronjob使每分钟执行一次。
此脚本的作用是：利用iptables屏蔽每分钟访问页面超过20的IP，这些页面已经排除图片,css,js等静态文件。

第二个脚本是通过在日志中查找cc攻击的特征进行屏蔽。

```bash
#!/bin/bash
keyword="cc-atack"
badip=`tail -n 5000  /home/www.centos.bz/log/access.log | grep "$keyword"  | awk '{print $1}' | sort | uniq -c | sort -nr | awk '{print $2}'`
if [ ! -z "$badip" ];then
for ip in $badip;
do
if test -z "`/sbin/iptables -nL | grep $ip`";then
/sbin/iptables -I INPUT -s $ip -j DROP
fi
done
fi
```
keyword则是日志中cc的特征，替换成有效的即可。