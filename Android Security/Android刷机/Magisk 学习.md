# Magisk 学习

src:https://www.coolapk.com/feed/5635088



又想ota，又想修改系统，还想卸载系统预装，怎么办？推荐一款神器：Magisk，既能ota又能修改系统。

# 1. Magisk 与 Xposed
很多人有误解，xp和框架和Magisk是不是一类的？工作修改原理是差不多的，但是工作目录路径不同，简言之：
1. xp框架的模块是通过查找原软件的代码，进行拦截，然后释放自身模块的代码到软件内，工作环境是软件，这也就是为何有些软件升级失效，因为软件代码被改了，通过拦截代替达不到效果！
2. Magisk框架的模块是通过查找安卓文件目录的文件，进行拦截，然后释放自身模块的文件到安卓目录下，工作环境是安卓目录
3. 他们相同的工作方式通过拦截隐藏源文件，然后进行替换！而源文件还在

通过这个原理，那么我们就可以用Magisk来改系统了，系统ota验证一般安卓目录分区，还有内核，我们一般动也是动这两分区
1. 最简单的方法就是，软件内点击卸载还原boot分区，然后关机就能ota了
2. 进入`/data/stock_boot_85f6e87d9f00375e612f51267cd8db94b0fab263.img.gz`，把这个gz解压出来，会有一个boot，rec刷入boot，就能ota了，ota后再刷入Magisk就行了

# 2. Magisk 原理
首先简单介绍一下magisk的原理。magisk做的事情是通过boot中创建钩子，进而bind mount构建出一个在system基础上能够自定义替换，增加以及删除的文件系统，实际上并没有对 system 分区进行修改（即 systemless 接口，以不触动 system 的方式修改 system）。所有操作都在启动的时候完成，启动过程中magisk所做的事情：
1. 准备阶段，将会把`/data/magisk.img`挂到`/magisk`。同时它会遍历magisk目录中的模块是否为启用状态，并且记录。
2. 创建骨架system文件系统(由于`bind mount `必须要有一个目标文件才能进行`bind mount`)，全部由`mkdir`和`touch`构建
3. 将每个标记为启用的`/magisk/$MODID/system`中文件`bind mount`到骨架系统
4. 执行自定义模块中的脚本
5. 遍历骨架中剩余没有被mount的文件，通过真正的system文件进行`bind mount`。

# 3. Magisk 可以做什么
然后Magisk能实现哪些功能？（ 截至目前版本（v16.0BETA））
1. 集成了root功能（MagiskSU）
2. 类似于Xposed，可以安装使用Magisk相关模块
3. `Magisk Hide`（对单个应用隐藏 Magisk 的 root 权限）
4. 日志功能
5. `Systemless hosts`（为广告屏蔽应用提供`Systemless hosts`支持）
6. `SafetyNet` 检查功能

Magisk对系统不起作用，因为它不会以任何方式改变您的system分区。这意味着您仍然可以安装官方OTA更新，而不会丢失root。而且MagiskSU在某些方面其实比SuperSU好得多（SuperSU已经商业化，后续开发的版本还能不能用自行斟酌）。难怪它成为定制ROM开发人员的默认选择。而且目前Magisk支持的系统为Android5.0+，最高支持安卓8.1。

# 4. Magisk 模块
不过目前Magisk不能与发展了多年的Xposed相比，比较实用的模块相对比较少，但是还是看到有很多开发者加入到了Magisk模块的开发，不过Magisk也兼容Xposed。相信慢慢Magisk将会更实用。
推荐的模块： （大部分可以在Magisk自带下载的仓库找到）)
1. Systemless Xposted V89.2(SDK21-25)
安卓8.0和安卓8.1是v90.3 (软件内下载)
Magisk版的XP和正式版的XP区别是mg版的是虚拟挂载，不修改system分区。管理起来更加方便。
2. Busybox Magisk
将BusyBox安装到设备的架构（x86 / ARM / ARM64）
3. [ViPER4Android FX ](http://t.cn/RK9ejRn)
蝰蛇音效不多介绍了吧，用mg装这个还是挺方便的
4. [Dolby Atoms R6.5](http://t.cn/Rj4QZg2 )
个人感觉杜比的外放音效比蝰蛇好多了
5. [Greenify4Magisk[v3.9.6] ](http://t.cn/Rj4QZsT)
可以开启绿色守护的特权模式
6. [AD-Free Youtube with Background Play ](http://t.cn/Roh3JNt)
见名知意，屏蔽油管的广告，并且实现了Youtube Red的后台播放功能
7. [App Systemizer](http://t.cn/RMc72tx)
能把用户 App 挂载为系统 App 的模块，如黑域，钛备份等
8. Magisk_Manager_for_Recovery_Mode
能在rec下进行Magisk模块的卸载和挂载使用，测试模块必备，减少卡米卸载Magisk框架
https://pan.baidu.com/s/1o9yN3WA
9. 其他扩展(自己认为有用的) 链接: https://pan.baidu.com/s/1dGl8Fsd 密码:gzih