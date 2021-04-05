# 修改Nexus5的boot.img - 打开系统调试

url：https://eternalsakura13.com/2018/02/04/trick2/



## 使用须知

### 工具备注：

mkbootimg和unpackbootimg可能只能用在linux x86的系统上，那我的linux x64怎么能使用这两个工具，需要下载支持x86程序运行的库。
你可以参考这些命令去下载运行库。

```
sudo apt-get -y update
sudo apt-get -y upgrade
sudo apt-get -y install binutils nasm
sudo apt-get -y install gcc-multilib g++-multilib
sudo apt-get -y install libc6-dev-i386
```

### OS

linux上使用，mac和windows均不可以。
在ubuntu14.04 64位测试通过。

### boot.img

我编译好了两个版本的，Nexus5手机，[android4.4.4](https://github.com/eternalsakura/ctf_pwn/blob/master/boot/newboot4.4.4.img)和[android5.0.1](https://github.com/eternalsakura/ctf_pwn/blob/master/boot/newboot5.0.1.img)。
实体机测试通过，需要自取。

## 下载

下载[工具包](https://github.com/eternalsakura/ctf_pwn/tree/master/boot)和[android源码(根据自己的android版本)](https://developers.google.com/android/images)

## 解压源码，找到boot.img

## 将工具包的路径加入环境变量

`export PATH=/home/sakura/工具:$PATH`
将上面的/home/sakura/工具替换成你自己的工具包路径。

这样工具包里的程序就可以使用了。

## boot.img解包

`split-bootimg.pl boot.img`

## 处理boot.img-ramdisk.gz

运行下面的命令，对boot.img-ramdisk.gz进行解压：

```
mkdir ramdisk
cd ramdisk
gzip -dc ../boot.img-ramdisk.gz | cpio -i
```



## 修改default.prop，打开系统调试标志

找到解压出来的default.prop文件，将其中的ro.debuggable=0修改为ro.debuggable=1


## ramdisk目录打包

返回ramdisk的上层目录
`cd ..`
输入命令：
`mkbootfs ./ramdisk | gzip > ramdisk.img`

## 打包出新的boot.img

命令：

```
mkbootimg --base 0x00000000 --ramdisk_offset 0x02900000 --second_offset 0x00F00000 --tags_offset 0x02700000 --cmdline 'console=ttyHSL0 androidboot.hardware=hammerhead user_debug=31 maxcpus=2 msm_watchdog_v2.enable=1 earlyprintk' --kernel boot.img-kernel --ramdisk ramdisk.img -o newboot.img
```



## 将新的boot.img刷入手机

`adb reboot bootloader`
`fastboot oem unlock`
`fastboot flash boot newboot.img`
`fastboot reboot`


## 确认刷入成功,debuggable修改为1

```
adb shell
getprop | grep debuggable
```