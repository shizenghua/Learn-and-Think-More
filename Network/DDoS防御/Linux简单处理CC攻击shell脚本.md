# Linux简单处理CC攻击shell脚本

src:https://www.centos.bz/2012/06/linux-cc-attack-shell-script/



第一个脚本是通过查找日志中访问次数过多的ip,并用iptables屏蔽,600秒解封。

```bash
#!/bin/bash
btime=600
attacks=20
tmpBlockIPFile=/home/tmp_block_ip
timestamp=$(date +%s)
logPath="/home/ban.log"
 
#start detect bad ip
badip=`tac /home/www.centos.bz/access.log  | awk '
BEGIN{
cmd="date -d \"1 minute ago\" +%H%M%S"
cmd|getline a
}
{
$4 = substr($4,14,8)
gsub(":","",$4)
$4=$4+0
a=a+0
if ($4>=a){
print $1,$7
} else {
exit;
}
}' | egrep -v '\.(gif|jpg|jpeg|png|css|js)' | awk '{print $1}' | sort | uniq -c | awk -v t="$attacks" '{$1=$1+0;t=t+0;if ($1>=t) print $2}'`
 
if [ ! -z "$badip" ];then
    for ip in $badip;
    do
        if test -z "`/sbin/iptables -nL | grep $ip`";then
            /sbin/iptables -I INPUT -s $ip -j DROP
 
            #record blocked ip
            echo "$timestamp $ip" >> $tmpBlockIPFile
            echo "$(date) $ip" >> $logPath
        fi
    done
fi
 
#unblock ip
if [ -f "$tmpBlockIPFile" ];then
    ips=""
    while read blockTime ip
    do
        ((interval=$timestamp - $blockTime))
        if [ $interval -gt $btime ];then
            ips="$ips $ip\n"
        fi   
    done < $tmpBlockIPFile
 
    if [ "$ips" != "" ];then
        for ip in `echo -e $ips`
        do
            sed -i "/$ip/d" $tmpBlockIPFile
            /sbin/iptables -D INPUT -s $ip -j DROP
        done
    fi
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