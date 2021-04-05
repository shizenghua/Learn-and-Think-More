# Windows下设置git代理

url：https://kaige.org/2020/02/11/proxy-setting-for-git-on-Windows/

### 适用场景

- Windows 10
- Git Bash 下的 git 命令

### 结论

- 参考：[Stack Overflow: SSH in git behind proxy on windows 7](https://stackoverflow.com/questions/5103083/ssh-in-git-behind-proxy-on-windows-7)
- 在 `~/.ssh/config` 中添加：

```
Host github.com
        User git
        IdentityFile ~/.ssh/id_rsa
        # ProxyCommand nc -X 5 -x 127.0.0.1:1080 %h %p # if `nc` is installed
        ProxyCommand connect -S 127.0.0.1:1080 %h %p # for git bash on windows
```

### 过程

国内连github慢，需要加个代理，以前记得使用`ALL_PROXY=socks5://127.0.0.1:180`前缀再执行git类命令是可以走代理的，这次使用git pull一直不行。
使用`Microsoft Network Monitor`监控流量，发现启动了也给ssh.exe进程，看github官网描述，
[git@github.com](mailto:git@github.com)类的地址是走ssh协议的，故而需要配ssh的config，发现之前已经弄过类似的，即通过ProxyCommand挂上nc命令，但Git Bash的MinGW环境没有nc，
后来就找到上面的参考链接，知道是`connect`可以替代。

之前看网上类似配置`git config --global http.proxy`和`https.proxy`的，配了没用，应该就可以理解了，没有走这俩协议，走的ssh。