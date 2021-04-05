# 执行ssh-add时出现Could not open a connection to your authentication agent

若执行ssh-add /path/to/xxx.pem是出现这个错误:Could not open a connection to your authentication agent，则先执行如下命令即可：

　　**ssh-agent bash**

***

更多关于ssh-agent的细节，可以用 man ssh-agent 来查看

