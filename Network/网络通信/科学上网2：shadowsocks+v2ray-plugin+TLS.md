# 科学上网2：shadowsocks+v2ray-plugin+TLS

src:https://huhao.ai/shadowsocks-v2ray-plugin-tls-cdn/

![科学上网2：shadowsocks+v2ray-plugin+TLS](images/shadowsocks_v2ray-plugin_tls.jpg)

# 一、背景

直接使用shadowsocks已经连不上主机了，需要对流量进行混淆。之前使用simple-obfs，一直能用，不过由于已经停止更新，社区都推荐大家升级到v2ray-plugin。

本文记录了下在vultr主机上快速搭建基于docker的shadowsocks+v2ray-plugin的过程。

机器配置：

- CentOS 8
- 境外VPS：统一用 1xx.1xx.xxx.147 表示

**文档更新历史**

- 2020.5.10 初次发表

# 二、技术科普

## 用到的工具

- v2ray：一款比shadowsocks更普适的流量转换器
- v2ray-plugin：基于v2ray原理，工作在shadowsocks上的流量混淆插件
- caddy：一款自动管理TLS证书的web server，替代nginx

## 流量混淆的技术原理

想要了解原理，用wireshark抓包看一下就很清楚了：

- 原始的基于Websocket的数据传输，特征非常明显：有大量数据传递是在Websocket协议包里。
- 使用HTTP协议的数据传输：第一个包是HTTP协议包，后面的数据传递都是普通的TCP包。
- 流量混淆伪装成HTTP：Websocket协议只用来协商必要的内容同步信息，真实的数据传递都是用普通的TCP包（看起来像是http协议在传递数据）

因此，使用v2ray-plugin插件，在数据包层面个的效果就是：Websocket包、TCP包混合通信。

### 网络结构（对应升级方案2）

```
                                                            ||
                                                            ||
                                                       GFW  ||
                                                            ||           sec.huhao.ai:443
                                                    TLS handshake       +-----------+-----+
                                                    |-------||--------->| TLS cert  |     |
                                                    |       ||          +-----------+     |
                                                    --------||--------->|     caddy       |
                                                    |  websocket + tcp  +-----------------+
                                                    |       ||                   |
                                                    |       ||                   |
                                                    |       ||                   |
                                                    |       ||                   |websocket + tcp
                                                    |       ||                   |
                                                    |       ||                   |
                                                    |       ||                   |
+------------+            +----------------+        |       ||          +--------v--------+            +------------+
|            | websocket  |                |        |       ||          |                 | websocket  |            |
|  ss-local  |----------->  v2ray-plugin   |--------+       ||          |  v2ray-plugin   |------------>  ss-server |
|            |            |                |                ||          |                 |            |            |
+------------+            +----------------+                ||          +-----------------+            +------------+
127.0.0.1:1086              127.0.0.1:XXXX                  ||          127.0.0.1:12785                127.0.0.1:YYYY
                                                            ||
                                                            ||
                                                            ||
```

# 三、详细步骤

## Docker安装和测试

### Step1: 安装docker

```shell
$ yum-config-manager --add-repo=https://download.docker.com/linux/centos/docker-ce.repo
$ dnf install https://download.docker.com/linux/centos/7/x86_64/stable/Packages/containerd.io-1.2.6-3.3.el7.x86_64.rpm
$ dnf -y install docker-ce
```

Shell

Copy

### Step2: 启动docker

```shell
$ systemctl enable --now docker
$ systemctl is-active docker
$ systemctl is-enable docker

#关闭防火墙
$ systemctl disable firewalld
```

Shell

Copy

### Step3: 快速测试docker

快速启动一个http服务在9000端口

```shell
$ docker run --rm --name=linuxconfig-test -p 9000:80 httpd
```

Shell

Copy

直接在服务器上访问这个http服务查看结果

```shell
$ curl 127.0.0.1:9000
```

Shell

Copy

## shadowsocks + v2ray-plugin安装和测试

### Step1: 设置shadowsocks配置文件

```shell
$ cat /etc/shadowsocks-libev/docker_v2ray_http/config.json
{
    "server":"0.0.0.0",
    "server_port":8588,
    "method":"aes-256-gcm",
    "password":"ss_password_example",
    "timeout":300,
    "fast_open":true,
    "nameserver":"8.8.8.8",
    "mode":"tcp_and_udp",
    "plugin":"v2ray-plugin",
    "plugin_opts":"server"
}
```

Shell

Copy

配置文件说明：

- plugin_opts的参数是传给插件v2ray-plugin的
- 此处的server是v2ray-plugin的参数，相当于以server模式运行
- mode: tcp_and_udp表示同时支持tcp和udp代理

## Step2: docker启动shadowsocks + v2ray-plugin

```shell
#这是teddsun制作的一个包含shadowsocks和v2ray-plugin的镜像
$ docker pull teddysun/shadowsocks-libev

$ docker run -d \
    --name ss-libev-http \
    --rm \
    -p 12782:8588 \
    -p 12782:8588/udp \
    -v /etc/shadowsocks-libev/docker_v2ray_http:/etc/shadowsocks-libev \
    teddysun/shadowsocks-libev
```

Shell

Copy

参数说明：

- 镜像teddysun/shadowsocks-libev默认使用8588端口监听

## Step3: 客户端连接

**操作方式1:命令行操作**

```shell
$ cat ss-local-config.json
{
  "timeout" : 60,
  "server" : "1xx.1xx.xxx.147",
  "server_port" : 12782,
  "password" : "ss_password_example",
  "local_address" : "127.0.0.1",
  "method" : "aes-256-gcm",
  "local_port" : 1086,
  "plugin" : "plugins/v2ray-plugin",
  "plugin_opts" : ""
}

$ ./ss-local -c ss-local-config.json --fast-open -u -v --reuse-port
```

Shell

Copy

参数说明：

- plugin参数指定v2ray-plugin的运行路径

**操作方式2:GUI操作**

Macos下通过ShadowsocksX-NG操作

```shell
地址: 1xx.1xx.xxx.147
Port: 12782
加密方法: aes-256-gcm
密码: ss_password_example
插件: v2ray-plugin      #可以打开插件目录，查看插件名称是否一致
插件选项: （留空）
```

Shell

Copy

## Step4: 测试

```shell
#服务端查看日志
$ docker ps -f ss-libev-http

#客户端指定代理下载文件
$ curl --socks5-hostname 127.0.0.1:1086 http://cachefly.cachefly.net/100mb.test > /dev/null
```

Shell

Copy

# 四、升级方案1：使用TLS加密传输数据

## 升级的技术原理

使用普通模式的v2ray-plugin，数据通过TCP包进行传递，虽然内容上已经通过指定的加密方法加密，比如aes-256-gcm，但是安全性性能依然不够。加密秘钥、方式固定不变，从长期来看，被嗅探的风险依然很大。

升级原理类似HTTPS使用TLS协议加密传输数据，让v2ray-plugin也使用TLS协议替代直接的TCP协议传输数据。(当然，TLS的底层依然是TCP)

使用TLS协议需要服务器上有CA颁发的域名证书，这个是由TLS层加密通信的机制决定的。TLS建立通信需要验证服务端公钥，而公钥必须放在CA认证的证书里才能有效。此外，所有CA颁发的证书都有有效期，一般是3个月，由于有效期短相对安全性更好，因此，未来趋势是越来越短。

因此，升级成TLS加密传输的必备条件：

- 需要1个自己的域名，比如这里我的域名是 huhao.ai
- 需要一套自动申请、管理域名证书的机制（自动更新证书有效期）

## Step1: 使用ACME.sh获取证书

ACME是由Let’s Encrypt CA指定的一套证书自动化证书管理环境的协议，已经在2019年被纳入互联网标准。
Let's Encrypt CA通过ACME协议来认证域名所有权、签发免费的证书。

支持ACMEv2协议的有很多客户端，之前我用的是Let's Encrypt官方出的certbot，最近比较流行的是acme.sh。(没有看错，这个工具是用shell脚本写的，支持各个平台，是很好的shell脚本语言学习材料)

**安装**

```shell
$ curl https://get.acme.sh | sh
$ source ~/.bashrc
```

Shell

Copy

说明：

- 这个脚本安装之后，默认会放在 /root/.acme.sh/acme.sh，同时生成一个 acme.sh的 alias command，对应的就是这个脚本

参考

- [acme.sh github](https://github.com/acmesh-official/acme.sh)

**获取域名服务商的API_KEY**

acme.sh脚本需要验证你有域名的所有权，因此，它需要直接跟域名服务商的服务器交互。大部分的域名服务商都已经支持ACMEv2协议，比如大家用的阿里云、百度云。

我这里用的是国外的CDN厂商CloudFlare作为域名服务商，方便后面做CDN服务（域名服务商可以任意转，不过转一次域名服务商生效时间比较久，一般都在24h~48h左右生效）

这里重点讲一下如何使用acme.sh签发CloudFlare管理的域名证书

CloudFlare的API key有两种使用方式

- 方式1：使用Global API Key + email 地址 （这个方式不推荐，因为Global API Key权限太大）
- 方式2：使用Token + Account ID（推荐）

Global API Key查看方法

- [cloudflare - profile - api tokens](https://dash.cloudflare.com/profile/api-tokens) 下方位置 「Global API Key - View」

Token的生成方法

- [cloudflare - profile - api tokens](https://dash.cloudflare.com/profile/api-tokens) 页面，「Create Token」
- 添加read access to Zone.Zone, and write access to Zone.DNS, across all Zones 权限

Account ID的查看方法

- 进入[cloudflare - 具体域名（比如huhao.ai)](https://dash.cloudflare.com/)
- 在Overview页面的右下角 Account ID

参考

- [acme.sh指定dns api key](https://github.com/acmesh-official/acme.sh/wiki/dnsapi)
- [cloudflare token生成](https://blog.outv.im/2019/fix-cloudflare-token/)

**使用acme.sh签发证书**

```shell
#验证Token的有效性
$ curl -X GET "https://api.cloudflare.com/client/v4/user/tokens/verify" \
     -H "Authorization: Bearer 9J6fcxxxxxxxxxDCG2p-y9FYv1nsyYMKJ8jgJprB" \
     -H "Content-Type:application/json"

#将key放入环境变量，acme.sh会自动保存到文件
$ export CF_Token="9J6fcxxxxxxxxxDCG2p-y9FYv1nsyYMKJ8jgJprB"
$ export CF_Account_ID="1118fe92xxxxxxxxxxxxxx3b403df8c9"

$ acme.sh  --issue  --dns dns_cf -d sec.huhao.ai -k ec-256 --ecc
.....
[Mon Apr 20 16:08:38 UTC 2020] Your cert is in  /root/.acme.sh/sec.huhao.ai/sec.huhao.ai.cer
[Mon Apr 20 16:08:38 UTC 2020] Your cert key is in  /root/.acme.sh/sec.huhao.ai/sec.huhao.ai.key
[Mon Apr 20 16:08:38 UTC 2020] The intermediate CA cert is in  /root/.acme.sh/sec.huhao.ai/ca.cer
[Mon Apr 20 16:08:38 UTC 2020] And the full chain certs is there:  /root/.acme.sh/sec.huhao.ai/fullchain.cer

#安装证书到指定位置
$ acme.sh  --installcert  -d sec.huhao.ai --key-file /etc/letsencrypt/huhao.ai/sec.huhao.ai.key --fullchain-file /etc/letsencrypt/huhao.ai/sec.huhao.ai.crt
```

Shell

Copy

参数说明 & 注意事项

- k表示指定key的长度
- --ecc表示使用椭圆曲线加密证书Elliptic Curve Cryptography (ECC)
- letsencrypt对签发证书接口有频率限制，不要频繁尝试，会被限速

## Step2: 重新配置shadowsocks

先看一下v2ray-plugin的使用说明

```shell
$ plugins/v2ray-plugin --help
Usage of plugins/v2ray-plugin:
  -V	Run in VPN mode.
  -cert string
    	Path to TLS certificate file. Overrides certRaw. Default: ~/.acme.sh/{host}/fullchain.cer
  -certRaw string
    	Raw TLS certificate content. Intended only for Android.
  -fast-open
    	Enable TCP fast open.
  -host string
    	Hostname for server. (default "cloudfront.com")
  -key string
    	(server) Path to TLS key file. Default: ~/.acme.sh/{host}/{host}.key
  -localAddr string
    	local address to listen on. (default "127.0.0.1")
  -localPort string
    	local port to listen on. (default "1984")
  -loglevel string
    	loglevel for v2ray: debug, info, warning (default), error, none.
  -mode string
    	Transport mode: websocket, quic (enforced tls). (default "websocket")
  -mux int
    	Concurrent multiplexed connections (websocket client mode only). (default 1)
  -path string
    	URL path for websocket. (default "/")
  -remoteAddr string
    	remote address to forward. (default "127.0.0.1")
  -remotePort string
    	remote port to forward. (default "1080")
  -server
    	Run in server mode
  -tls
    	Enable TLS.
  -version
    	Show current version of v2ray-plugin
```

Shell

Copy

可以看到，服务端的v2ray-plugin配置，如果要启用tls的话，需要指定server、tls、cert、key、host五个参数。

```shell
$ cat /etc/shadowsocks-libev/docker_v2ray_tls/config.json
{
    "server":"0.0.0.0",
    "server_port":8588,
    "password":"ss_password_example",
    "timeout":300,
    "method":"aes-256-gcm",
    "fast_open":true,
    "nameserver":"8.8.8.8",
    "mode":"tcp_and_udp",
    "plugin":"v2ray-plugin",
    "plugin_opts":"server;tls;host=sec.huhao.ai;cert=/opt/sec.huhao.ai.crt;key=/opt/sec.huhao.ai.key"
}
```

Shell

Copy

参数说明：

- cert、key的路径均为docker容器路径，需要在服务器启动docker的时候做映射

## Step3: docker启动基于TLS的shadowsocks + v2ray-plugin

```shell
$ docker run -d \
    --name ss-libev-tls \
    --rm \
    -p 12783:8588 \
    -p 12783:8588/udp \
    -v /etc/shadowsocks-libev/docker_v2ray_tls:/etc/shadowsocks-libev \
    -v /etc/letsencrypt/huhao.ai:/opt \
    teddysun/shadowsocks-libev
```

Shell

Copy

参数说明:

- 将server的/etc/shadowsocks-libev/docker_v2ray_tls目录映射到容器teddysun/shadowsocks-libev的/etc/shadowsocks-libev
- 将server的/etc/letsencrypt/huhao.ai目录映射到容器的 /opt目录，这样配置文件中指定的tls的key和cert就可以生效了

## Step4: 客户端连接

**操作方式1:命令行操作**

```shell
$ cat ss-local-config.json
{
  "timeout" : 60,
  "server" : "1xx.1xx.xxx.147",
  "server_port" : 12783,
  "password" : "ss_password_example",
  "local_address" : "127.0.0.1",
  "method" : "aes-256-gcm",
  "local_port" : 1086,
  "plugin" : "plugins/v2ray-plugin",
  "plugin_opts" : "tls;host=sec.huhao.ai"
}

$ ./ss-local -c ss-local-config.json --fast-open -u -v --reuse-port
```

Shell

Copy

**操作方式2:GUI操作**

Macos下通过ShadowsocksX-NG操作

```shell
地址: 1xx.1xx.xxx.147
Port: 12783
加密方法: aes-256-gcm
密码: ss_password_example
插件: v2ray-plugin
插件选项: tls;host=sec.huhao.ai
```

Shell

Copy

# 五、升级方案2：使用Caddy管理TLS证书

升级方案1的缺点是配置过程比较麻烦，需要使用acme.sh来签发证书。对于懒人来说，最好证书签发、管理的事完全不掺和。

Caddy是基于go的一款天然支持ACME协议的轻量级服务器，天生支持HTTPS，对于个人博主来说，简直是福音。以前需要使用certbot或者acme.sh管理证书，还要自己配置nginx服务器，现在都不用了。

Caddy只需要在启动的时候指定要搭建的网站域名，就可以自动通过ACME协议验证域名所有权、联系Letsencrypt签发证书，并且自动将所有http的请求转发到https，确实是「一键HTTPS」。

升级方案2使用caddy作为TLS证书管理器，v2ray-plugin不再需要启用tls，caddy完成tls认证之后将流量反向代理到v2ray-plugin即可。

整个方案的架构图本文最开始的图所示。

## Step1: 安装Caddy并启动（替代nginx）

**安装Caddy**

```shell
$ yum-config-manager --add-repo https://copr.fedorainfracloud.org/coprs/g/caddy/caddy/repo/epel-8/group_caddy-caddy-epel-8.repo
$ yum install caddy

$ caddy version
v2.0.0-rc.3 h1:z2H/QnaRscip6aZJxwTbghu3zhC88Vo8l/K57WUce4Q=   #caddy v2跟v1的配置文件有差别
```

Shell

Copy

**配置caddy**

```shell
$ cat /etc/caddy/Caddyfile
sec.huhao.ai {
        encode gzip
        reverse_proxy /ray 127.0.0.1:12785 {
                header_up -Origin
        }
}
```

Shell

Copy

参数说明

- 这里配置了一个sec.huhao.ai域名下的反向代理，所有请求  sec.huhao.ai/ray 的请求都会被转发到本地的 127.0.0.1:12785 地址。这个地址也就是后续我们的 shadowsocks+v2ray_plugin要监听接收请求的地址

**启动caddy**

```shell
$ systemctl enable --now caddy  #启动caddy服务
```

Shell

Copy

**检查caddy服务状态**

```shell
$ systemctl status caddy        #查看caddy服务状态
$ journalctl --no-pager -u caddy -f    #查看caddy服务的日志
```

Shell

Copy

从日志中，可以看到，caddy在第一次启动的时候，会自动从letsencrypt申请sec.huhao.ai的证书并保存在本地。这个过程中悄无声息的完成了对域名sec.huhao.ai的证书签发（包括到期自动更新）

## Step2: 配置并启动shadowsocks+v2ray_plugin(非TLS模式)

```shell
$ /etc/shadowsocks-libev/docker_v2ray_caddy/config.json
{
    "server":"0.0.0.0",
    "server_port":8588,
    "password":"ss_password_example",
    "timeout":300,
    "method":"aes-256-gcm",
    "fast_open":true,
    "nameserver":"8.8.8.8",
    "mode":"tcp_and_udp",
    "plugin":"v2ray-plugin",
    "plugin_opts":"server;path=/ray"
}
```

Shell

Copy

参数说明：

- 在v2ray_plugin的参数中，只需要填写server和path即可，**不要加tls**。因为tls的认证已经交给caddy了，v2ray_plugin只需要处理caddy转发过来的非TLS认证数据进行处理即可。
- path参数跟caddy反向代理路径保持一致

在本地的12785端口启动shadowsocks+v2ray_plugin，这个端口就是caddy转发的端口

```shell
$ docker run -d \
    --name ss-libev-caddy \
    --rm \
    -p 12785:8588 \
    -p 12785:8588/udp \
    -v /etc/shadowsocks-libev/docker_v2ray_caddy:/etc/shadowsocks-libev \
    teddysun/shadowsocks-libev
```

Shell

Copy

## Step3: 客户端配置

操作方式1:命令行操作**

```shell
$ cat ss-local-config.json
{
  "server" : "sec.huhao.ai",
  "server_port" : 443,
  "password" : "ss_password_example",
  "method" : "aes-256-gcm",
  "timeout" : 60,
  "local_address" : "127.0.0.1",
  "local_port" : 1086,
  "plugin" : "plugins/v2ray-plugin",
  "plugin_opts" : "tls;host=sec.huhao.ai;path=/ray"
}

$ ./ss-local -c ss-local-config.json --fast-open -u -v --reuse-port
```

Shell

Copy

**操作方式2:GUI操作**

Macos下通过ShadowsocksX-NG操作

```shell
地址: sec.huhao.ai
Port: 443
加密方法: aes-256-gcm
密码: ss_password_example
插件: v2ray-plugin
插件选项: tls;host=sec.huhao.ai;path=/ray
```

Shell

Copy

使用caddy作为反向代理，管理TLS证书的方式下，客户端参数配置说明：

- 服务器地址直接填写caddy的 域名地址 sec.huhao.ai，端口就是HTTPS端口443，caddy自动将请求转发给v2ray-plugin
- **客户端的v2ray-plugin参数依然需要填tls**，因为跟caddy交互还是需要TLS认证

## 域名解析的坑

使用Caddy作为TLS证书交互服务器，Shadowsocks+v2ray_plugin作为机场的方式，由于客户端并非直接指定服务器ip地址，而是指定了sec.huhao.ai的域名作为服务器地址，对于sec.huhao.ai的域名存在一个域名地址解析的问题。

- 如果本机/局域网内没有做域名过滤之类的任何处理，则默认sec.huhao.ai(放在国外域名服务商CloudFlare上)的解析在没有被污染的情况下，则不会有问题，上述配置可以生效。
- 如果本机通过类似DNSMasq+ChinaDNS或者其他自建的域名服务器，则需要将sec.huhao.ai的域名解析加入到白名单，也就是直接使用国内域名服务器解析。否则会出现循环解析sec.huhao.ai的问题，出现的现象是：
- shadowsocks服务器的**caddy日志，经常看不到任何访问信息**，或者是偶尔有、偶尔没有，GFW随机过滤
- shadowsocks服务器的**caddy日志，出现大量的TLS timeout超时错误**，因为域名解析的连接请求被GFW随机过滤导致时断时续
- 后续如果想让sec.huhao.ai的解析放到cloudflare的cdn上，也会遇到这个问题

**解决方案**

> 本地搭建域名解析服务器，可以参考DNSMasq+ChinaDNS的方案，将ss服务器域名(这里是sec.huhao.ai)放入白名单，通过国内域名服务器解析。

# 六、升级方案3：开启CDN，拯救被墙的shadowsocks

## 方案说明

此方案建立在方案2的基础上，需要服务器域名sec.huhao.ai处于CDN厂商管理下。cloudflare是一家可选的CDN服务商，一方面国内域名服务器对cloudflare的域名解析没有完全封闭，一方面cloudflare有免费的CDN流量。

原理很简单，上面已经解释过，方案2直接连接了caddy服务器 sec.huhao.ai的443端口，这个域名如果放在CDN上，并由域名服务器进行解析，就可以隐藏真实的SS服务器地址。

在上面的方案2完全work的情况下，具体做法：

- 开启cloudflare的CDN
- 设置cloudflare的SSL/TLS中的连接模式为Full
- 注意不要选择Full(strict)，因为我们sec.huhao.ai的证书是自己通过caddy管理的，而cloudflare做cdn还会生成一个证书。如果选strict模式，则需要把我们服务器上的证书换成cloudflare的证书。
- 之前在caddy v1的版本上，直接通过tls cloudflare参数可以实现这个。新版本的v2上，这个配置只能用json格式的配置文件，还没有来得及研究。

## 问题说明

使用CDN的目的并不是加速翻墙，主要是为了通过CDN隐藏被墙的ip。如果你的服务器ip被墙了，可以尝试这种方法，否则完全没有必要。

缺点如下:

- 由于CDN多了一层解析，翻墙速度减慢。以我的机器实测，本来SS服务器在TOKYO，TTL是100ms，使用cloudflare，本机到CF解析的美国地址的TTL是220ms，CF再回源取TOKYO的内容，又是100ms，慢了3倍。
- 此方法并不一定有效，因为CDN服务商给到的解析地址很可能被GFW墙了，从而达不到本方案的目的。我这边看到cloudflare把我的服务器解析到这个地址104.28.25.205，基本上访问不到。

# 七、其他问题

## docker0: iptables: No chain/target/match by that name.

解决方法：

```shell
$ service docker restart
$ iptables -L             #查看结果
```

Shell

Copy

原因：在启动firewalld之后，iptables被激活，此时没有docker chain，重启docker后被加入到iptable里面。
重启docker后：可以看到iptables里面多出了Chain Docker的选项。

## TLS handshake error (read: connection reset by peer  或者 read: connection

timed out)
原因:

- TLS的证书配置不正确，比如之前自己使用acme.sh申请了证书，后又使用caddy申请了一套。
- v2ray-plugin配置不正确，使用caddy转发的情况下不需要配置tls
- 请求被GFW随机过滤了，尤其是服务器域名请求

**解决方案**

- 将caddy管理的证书路径作为v2ray-plugin的证书参数的路径
- 考虑在本地搭建域名服务器，可以使用DNSMasq+ChinaDNS

# 参考

- [Shadowsocks基于二次混淆加密传输的数据保密性原理分析](https://www.secrss.com/articles/18469)
- [毕世平-使用 Docker 快速部署 Shadowsocks-libev + v2ray-plugin - 20191128](https://teddysun.com/569.html)
- [linuxconfig - how to install docker in rhel8](https://linuxconfig.org/how-to-install-docker-in-rhel-8)
- [letsencrypt接口限速说明](https://letsencrypt.org/docs/rate-limits/)
- [明月登楼网站建设心得分享](https://zhuanlan.zhihu.com/p/29507417)
- [wsgzao-使用 certbot 代替 acme.sh 免费申请 wildcard 通配符证书和自动更新实践小结-20200413](https://wsgzao.github.io/post/certbot/)
- [caddy install](https://caddyserver.com/docs/install)
- [shadowsocks官方的docker镜像](https://hub.docker.com/r/shadowsocks/shadowsocks-libev)



