# 优化LINUX内核阻挡SYN洪水攻击

src:https://www.centos.bz/2012/04/linux-syn/



SYN洪水攻击（SYN Flooding Attack）即是指利用了 TCP/IP 三次握手协议的不完善而恶意发送大量仅仅包含 SYN 握手序列数据包的攻击方式。该种攻击方式可能将导致被攻击计算机为了保持潜在连接在一定时间内大量占用系统资源无法释放而拒绝服务甚至崩溃。如果在Linux服务器下遭受SYN洪水攻击,可以进行如下一些设置:

#缩短SYN- Timeout时间：
iptables -A FORWARD -p tcp –syn -m limit –limit 1/s -j ACCEPT
iptables -A INPUT -i eth0 -m limit –limit 1/sec –limit-burst 5 -j ACCEPT

#每秒 最多3个 syn 封包 进入 表达为 ：
iptables -N syn-flood
iptables -A INPUT -p tcp –syn -j syn-flood
iptables -A syn-flood -p tcp –syn -m limit –limit 1/s –limit-burst 3 -j RETURN
iptables -A syn-flood -j REJECT

#设置syncookies：
sysctl -w net.ipv4.tcp_syncookies=1
sysctl -w net.ipv4.tcp_max_syn_backlog=3072
sysctl -w net.ipv4.tcp_synack_retries=0
sysctl -w net.ipv4.tcp_syn_retries=0
sysctl -w net.ipv4.conf.all.send_redirects=0
sysctl -w net.ipv4.conf.all.accept_redirects=0
sysctl -w net.ipv4.conf.all.forwarding=0
sysctl -w net.ipv4.icmp_echo_ignore_broadcasts=1

#防止PING：
sysctl -w net.ipv4.icmp_echo_ignore_all=1

#拦截具体IP范围：
iptables -A INPUT -s 10.0.0.0/8 -i eth0 -j Drop