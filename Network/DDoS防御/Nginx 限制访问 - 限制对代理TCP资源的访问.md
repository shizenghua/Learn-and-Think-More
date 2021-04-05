# Nginx 限制访问 - 限制对代理TCP资源的访问

src:[https://blog.csdn.net/kikajack/article/details/79339521?utm_medium=distribute.pc_aggpage_search_result.none-task-blog-2~all~sobaiduend~default-1-79339521.nonecase&utm_term=nginx%20%E6%9C%80%E5%A4%A7tcp%E8%BF%9E%E6%8E%A5&spm=1000.2123.3001.4430](https://blog.csdn.net/kikajack/article/details/79339521?utm_medium=distribute.pc_aggpage_search_result.none-task-blog-2~all~sobaiduend~default-1-79339521.nonecase&utm_term=nginx 最大tcp连接&spm=1000.2123.3001.4430)



本节提供了对基于 TCP 通信的数据库或 media 服务器的访问限制方案。 访问可以通过 IP 地址，同时连接数量或带宽进行限制。

# 1. 通过 IP 地址限制访问
Nginx 可以基于指定的客户端 IP 地址或地址段允许或拒绝访问。在 stream 上下文或 server 块中使用 allow 或 deny 指令可以允许或拒绝访问：

```
stream {
    ...
    server {
        listen 12345;
        deny   192.168.1.2;
        allow  192.168.1.1/24;
        allow  2001:0db8::/32;
        deny   all;
    }
}
```
规则从上到下按顺序处理：`如果队列中的第一个指令拒绝所访问，那么后面的所有 allow 指令无效。`这个例子中，允许来自子网段 192.168.1.1/24 的访问，但是 192.168.1.2 这个 IP 地址除外。IPv6 地址段中的 2001:0db8::/32 也被允许访问，除此之外所有的 IP 地址都被禁止了。

# 2. 限制 TCP 连接的数量
可以限制一个 IP 地址的 TCP 同时连接的数量。在防止 DoS （denial-of-service）攻击时这很有用。

首先，定义存储每个服务器的最大 TCP 连接数量的区域，定义关键字来标志连接。在 `stream` 上下文中使用 `limit_conn_zone` 指令即可实现：

```
stream {
    ...
    limit_conn_zone $binary_remote_addr zone=ip_addr:10m;
    ...
}
```

这个例子中用来标志每个连接的关键字是 `$binary_remote_addr`，可以用二进制格式表示 IP 地址。共享内存区域的名称是 `ip_addr`，大小是 10 MB。
在共享内存区域定义好后，可以通过 `limit_conn` 指令限制连接数。`limit_conn` 指令的第一个参数指明使用哪一个共享内存区域（之前用 `limit_conn_zone` 指令定义的）。第二个参数指明 stream 上下文或 server 块中对每一个 IP 地址允许的最大连接数：

```
stream {
    ...
    limit_conn_zone $binary_remote_addr zone=ip_addr:10m;

    server {
        ...
        limit_conn ip_addr 1;
    }
}
```
限制每个 IP 地址的连接数量时，注意在**每个 Network Address Translation （NAT）后面可能会有多个主机共享这个 IP**。

# 3. 限制带宽
可以配置 TCP 连接的最大下载或上传速度。分别引入 `proxy_download_rate` 或 `proxy_upload_rate` 指令即可：

```
server {
    ...
    proxy_download_rate 100k;
    proxy_upload_rate   50k;
}
```


通过这些设置，客户端每个连接下载数据的最大速度为每秒 100 KB，每个连接上传数据的最大速度为每秒 50 KB。然而，客户端可以打开读个连接。所以，如果想限制每个客户端的全局速度，每个客户端的连接数量也需要限制为 1。

```
stream {
    ...
    limit_conn_zone $binary_remote_addr zone=ip_addr:10m;

    server {
        ...
        limit_conn ip_addr 1;
        proxy_download_rate 100k;
        proxy_upload_rate   50k;
    }
}
```