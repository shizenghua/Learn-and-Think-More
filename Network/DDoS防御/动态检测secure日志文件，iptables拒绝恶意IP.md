# 动态检测secure日志文件，iptables拒绝恶意IP

src:http://blog.chinaunix.net/uid-346158-id-2130832.html



这个脚本的作用是定期检查系统secure日志，一旦发现有恶意IP在猜测SSH密码，则记录该IP并将之加入到iptables的DROP行列里。此脚本只检测非法用户登录失败的日志，如果用户是有效的不会检测。所以笔者建议禁止系统root用户SSH，并且你常用的帐户名设一个不容易被猜到的名字。

```bash
#!/bin/bash
# Author : Daihaijun. 2008-9-8
# Last Modify: 2008-9-11
# vi /var/spool/cron/root
# */10 * * * * /data/shell/chksship/chksship.sh.x
SECURE_LOG="/var/log/secure"
IPTABLES_CMD="/sbin/iptables"
SSHIPCHK_DIR="/var/log/chksship"
FAILEDIP="$SSHIPCHK_DIR/sshfailip"
DENYSSHIP_FILE="$SSHIPCHK_DIR/denysshiplist"
DENYSSHIP_LOG="$SSHIPCHK_DIR/denysshiplog"
TEMPFAILEDIP="$SSHIPCHK_DIR/tempfailip"
INPUTCHAIN="RH-Firewall-1-INPUT"
LOCKFILE="/var/run/chkssh.lock"
#
# If this script is running on ,exit,else create a lock file.
if [ -e "${LOCKFILE}" ];then
  echo "This script is running..."
  sleep 3
  exit 0
else
  touch ${LOCKFILE}
fi
#
if [ ! -d $SSHIPCHK_DIR ];then
   mkdir -p $SSHIPCHK_DIR
fi
#
if [ ! -f $SSHIPCHK_DIR/oldnum ];then
   touch $SSHIPCHK_DIR/oldnum
   echo 0 > $SSHIPCHK_DIR/oldnum
fi
if [ ! -f $DENYSSHIP_LOG ];then
   touch $DENYSSHIP_LOG
fi
#定义一个函数，取负数的绝对值
ABS()
{
echo $1 | awk '{print sqrt($1*$1)}';
}

OLD_LAST_LINE_NUM=`cat $SSHIPCHK_DIR/oldnum`

wc -l $SECURE_LOG |awk '{ print $1 }' > $SSHIPCHK_DIR/newnum

CUR_LAST_LINE_NUM=`cat $SSHIPCHK_DIR/newnum`

W=$(($CUR_LAST_LINE_NUM - $OLD_LAST_LINE_NUM))
Z=`ABS $W`
DYNAMIC_SCAN()
{
/usr/bin/tail -$Z $SECURE_LOG| awk -F: '/Failed/ { print $7 }' |cut -d" " -f1| \
sort -n|uniq -c|sort -rn > $TEMPFAILEDIP
/bin/awk '{ if ( $1>3 ) print $2 }' $TEMPFAILEDIP > $DENYSSHIP_FILE
/bin/echo "`date +%Y-%m-%d_%H:%M:%S`" >> $FAILEDIP
/bin/cat $DENYSSHIP_FILE >> $DENYSSHIP_LOG
/bin/cat $TEMPFAILEDIP >> $FAILEDIP
/bin/cat $SSHIPCHK_DIR/newnum > $SSHIPCHK_DIR/oldnum
}
```

下面这个函数的作用是如果DROP的IP达到100，则开始清空这些曾经被DROP的IP。

一来iptables表逐渐变得庞大影响效率；(以后考虑根据时间来释放被拒绝的IP)

二来这种类型的恶意IP都是短时间内攻击，即使再次恶意访问，仍旧会被脚本再次列入DROP规则里，所以清理是有好处的。

```bash
CLEAR_RULE()
{
DENYIP_RULE_LIST=`/sbin/iptables-save |awk '{ if ( $6=="DROP" ) print $4 }'|sed /^$/d`
DENYIP_RULE_NUM=`/sbin/iptables-save |awk '{ if ( $6=="DROP" ) print $4 }'|sed /^$/d|wc -l`

if [ "$DENYIP_RULE_NUM" -ge "100" ];then
  for dip in $DENYIP_RULE_LIST
  do
    $IPTABLES_CMD -D $INPUTCHAIN -s $dip -j DROP
  done
else
  /bin/echo "`date +%Y-%m-%d_%H:%M:%S`" >> $FAILEDIP
  rm -fr $LOCKFILE
  exit 0
fi
/bin/echo "`date +%Y-%m-%d_%H:%M:%S`" >> $FAILEDIP
}


if [ "$W" -eq "0" ];then
T=ZERO
fi

if [ "$W" -gt "0" ];then
T=GTZERO
fi

if [ "$W" -lt "0" ];then
T=LTZERO
fi

case "$T" in
     ZERO)
       CLEAR_RULE;;
     GTZERO)
       DYNAMIC_SCAN;;
     LTZERO)
       echo 0 > $SSHIPCHK_DIR/oldnum
       Z=$CUR_LAST_LINE_NUM
       DYNAMIC_SCAN;;
esac

for ip in `cat $DENYSSHIP_FILE` ; do
    if [ -z "`/sbin/iptables-save |grep $ip`" ];then
        $IPTABLES_CMD -I $INPUTCHAIN -s $ip -j DROP
    fi
#        /etc/init.d/iptables save
done
/bin/cat /dev/null > $DENYSSHIP_FILE
rm -fr $LOCKFILE
exit 0
############### Script End #################
```