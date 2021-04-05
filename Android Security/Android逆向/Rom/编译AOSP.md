# 编译AOSP

[2020年12月17日](https://wrlus.com/cyber-security/mobile/build-aosp/) [小路](https://wrlus.com/author/wrlu/)％1 $ S

## 一、AOSP简介

- Android是一个针对多种不同设备类型打造的开放源代码软件堆栈。Android 的主要目的是为运营商、OEM 和开发者打造一个开放的软件平台，使他们能够将创新理念变为现实，并推出能够卓有成效地改善用户移动体验的真实产品。
- Android平台的设计可确保不存在一个集中瓶颈，即没有任何行业参与者可一手限制或控制其他参与者的创新。这样，我们不但可以打造功能完善的高品质消费类产品，而且可以完全开放源代码，供第三方自由定制和移植。
- 简单点说，AOSP就是Android系统的开源版本，其中不包括各家OEM厂商的闭源驱动，也不包括GMS等，也就是俗称的“原生”系统。



## 二、Make和Soong编译系统

- 在 Android 7.0 发布之前，Android 仅使用 GNU Make 描述和执行其编译规则。Make 编译系统得到了广泛的支持和使用，但在 Android 层面变得缓慢、容易出错、无法扩展且难以测试。Soong 编译系统正提供了 Android 编译版本所需的灵活性。
- 对于实际的使用来说，Soong明显成功率和错误提示都更好。

## 三、AOSP源码的获取

- AOSP的源码可以通过Google的Git库：[android Git repositories – Git at Google](https://android.googlesource.com/) 获取，同时还要使用`repo`工具和`Linux`系统（最好是`Ubuntu`），获取AOSP源码至少需要100GB的可用磁盘空间。
- 因为众所周知的原因，在国内是无法访问上述地址的，所以我们选择使用USTC的镜像（当然也可以使用TUNA或者其他镜像）。
- 下载依赖`git`和`python`

```
sudo apt-get install git python
```

- 获取`repo`工具，如果是基于APT的系统，可以直接通过APT获取：

```
sudo apt-get install repo
```

- 或者也可以直接从USTC上获取，下载之后放入`/usr/bin`中，并给予执行权限即可

```
curl -sSL  'https://gerrit-googlesource.proxy.ustclug.org/git-repo/+/master/repo?format=TEXT' |base64 -d > ~/repo
sudo cp ~/repo /usr/bin/
sudo chmod +x /usr/bin/repo
```

- 如果在WSL上面同步代码，需要设置NTFS文件系统的区分大小写属性，否则会出现各种问题

```
fsutil.exe file SetCaseSensitiveInfo D:\aosp enable
```

- 然后可通过下面的链接获取月度包，月度包下载地址：http://mirrors.ustc.edu.cn/aosp-monthly/
- 解压月度包tar文件，然后进行同步即可：

```
tar xf aosp-latest.tar
cd aosp/
repo sync
```

- 也可以配置好REPO_URL之后然后直接初始化

```
export REPO_URL='https://gerrit-googlesource.proxy.ustclug.org/git-repo'
repo init -u git://mirrors.ustc.edu.cn/aosp/platform/manifest
repo sync
```

- 上面获取的是master分支，如果需要特定版本，则需要重新使用`repo init`命令进行指定，然后再进行同步：

```
repo init -u git://mirrors.ustc.edu.cn/aosp/platform/manifest -b android-11.0.0_r4
repo sync
```

- `-b`参数后面跟的是分支名称，参考以下网站：https://source.android.google.cn/setup/start/build-numbers?hl=zh-cn
- 如果切换到比较久远的分支，则会耗费比较长的时间来同步。当同步完成之后，就可以在`aosp/`目录下查看到AOSP的源码了。

## 四、Pixel专有软件的获取

- Google设备获取二进制文件的方法：
  - master分支：`https://developers.google.cn/android/blobs-preview`(仅支持最新设备，目前是Pixel 5系列、Pixel 4a系列、Pixel 4系列、Pixel 3a系列和Pixel 3系列)
  - release标签：`https://developers.google.cn/android/drivers`
- 每组二进制文件都是压缩包中的一个自解压脚本。解压每个压缩包，从源代码树的根目录运行附带的自解压脚本，然后确认您同意附带的许可协议的条款。二进制文件及其对应的 Makefile 将会安装在源代码树的 vendor/ 层次结构中。

## 五、AOSP源码的编译

- 如果说AOSP源码获取是取决于你的网速，那么编译就是取决于你的电脑配置了。
- 首先需要安装一些依赖

```
sudo apt-get install unzip m4 libncurse5 moreutils
```

- 然后要进行环境初始化，执行下面的命令：

```
source build/envsetup.sh
```

- 然后执行命令选择编译目标，根据实际情况选择，注意，如果需要在真机上运行，别忘了通过第四节的方法获取对应的驱动程序：

```
lunch
```

- 最后使用`m`命令进行编译，推荐使用`-j`参数指定线程数，一般可以设置为CPU逻辑核心数*2，例如我的电脑是`i5-8250U`，有4个CPU核心，共8个线程，所以可以使用以下命令进行编译：

```
m -j16
```

## 六、AOSP在模拟器中运行

- 编译完成后，输入

```
emulator
```

## 七、在真机上运行

- 如果在`lunch`时选择的是真机目标并且已完成了第四节的内容，则在编译完成后，也会编译好fastboot，输入下面的命令即可将镜像写入手机（需要手机已经解锁bootloader）：

```
# -w 代表清除/data分区，删除所有用户数据，如果不需要可不加
fastboot flashall -w
```

## 八、常见错误

- Q1：提示 os::fork_and_exec failed: Not enough space
  - A1：系统内存不足，一般可以选择调小一些线程再编译。
- Q2：提示 IO failure on output stream: No space left on device
  - A2：硬盘空间不足，一般来说编译一次至少要有250GB以上的空间