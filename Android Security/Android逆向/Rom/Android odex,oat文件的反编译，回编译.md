# Android odex,oat文件的反编译，回编译

原文链接：https://www.cnblogs.com/luoyesiqiu/p/11802947.html

现在，许多Android手机的ROM包在生成过程中都启用优化，把jar文件抽空，生成odex/oat和vdex文件，以在运行时省掉编译时间。如果想对这些jar进行修改，就要修改它们所对应的odex或者oat文件。本文以`/system/framework/oat/arm64/am.odex`为例，讲解它的反编译和回编译过程

本文用到的工具:baksmali.jar和smali.jar

下载地址：
https://bitbucket.org/JesusFreke/smali/downloads

## 将odex/oat反编译

在执行反编译前，先将odex/oat和它的依赖复制到同一目录下。如果在反编译odex/oat的时候，没有找到依赖，就会报类似于下面的错误：

```
Exception in thread "main" org.jf.dexlib2.DexFileFactory$DexFileNotFoundException: Could not locate the embedded dex file /system/framework/am.jar. Is the vdex file missing?
	at org.jf.dexlib2.dexbacked.OatFile$OatDexEntry.getDexFile(OatFile.java:586)
	at org.jf.dexlib2.dexbacked.OatFile$OatDexEntry.getDexFile(OatFile.java:567)
	at org.jf.baksmali.DexInputCommand.loadDexFile(DexInputCommand.java:158)
	at org.jf.baksmali.DisassembleCommand.run(DisassembleCommand.java:162)
	at org.jf.baksmali.Main.main(Main.java:102)
Caused by: org.jf.dexlib2.dexbacked.DexBackedDexFile$NotADexFile: Not a valid dex magic value: 7f 45 4c 46 02 01 01 03
	at org.jf.dexlib2.util.DexUtil.verifyDexHeader(DexUtil.java:93)
	at org.jf.dexlib2.dexbacked.OatFile$OatDexEntry.getDexFile(OatFile.java:583)
	... 4 more
```

(1) 在复制依赖前先挂载system.img到`mysystem`目录:

```
mkdir -p mysystem && sudo mount -o loop system.img mysystem
```

(2) 复制以boot开头的文件依赖

然后执行下面的命令复制依赖到当前目录：

```
cp mysystem/framework/arm64/* .
```

(3) 复制odex/oat本身及相关文件

```
cp mysystem/framework/oat/arm64/am.* .
```

> 这个路径不是死的，请根据实际情况复制。如果没有system.img镜像，或者无法挂载，也可以从手机把相关文件复制出来:`adb pull /system/framework`

(4) 反编译odex/oat

```
java -jar baksmali-2.3.4.jar x am.odex
```

- x deodex的缩写

反编译后，会生成`out`目录，里面存放反编译出来的代码，代码格式为smali

现在，你可以对里面的代码进行修改了

## 将smali编译成dex

修改完毕后，将out目录下smali代码编译成dex:

```
java -jar smali-2.3.4.jar as out/ -a 28 -o am.dex
```

- as assemble的缩写
- -a 指定API版本
- -o 指定输出的dex文件

如果没有错误就会生成am.dex文件。如果有错误就根据日志的内容去解决错误

## 将dex编译成odex/oat

dex编译成odex/oat是在手机上进行的，所以生成了dex文件以后，我们把am.dex推到手机：

```
adb push am.dex /data/local/tmp
```

让手机连接到电脑，打开USB调试，在命令行执行`adb shell`，接着输入下面的命令进行编译：

```
export ANDROID_DATA=/data
export ANDROID_ROOT=/system
dex2oat --dex-file=/data/local/tmp/am.dex --oat-file=/data/local/tmp/am.odex  --instruction-set=arm64 --runtime-arg -Xms64m --runtime-arg -Xmx128m
```

- --dex-file 指定要编译的dex文件
- --oat-file 指定要输出的odex/oat文件
- --instruction-set 指定cpu架构
- --runtime-arg 指定dex2oat运行时的参数，如果编译时发生内存不足，可以把Xms和Xmx调大

编译成功后，会在/data/local/tmp目录生成odex/oat和vdex文件，将它们替换到系统试试吧







## 错误应对

```
Error occurred while loading class path files. Aborting.
org.jf.dexlib2.analysis.ClassPathResolver$ResolveException: org.jf.dexlib2.analysis.ClassPathResolver$NotFoundException: Could not find classpath entry boot.oat
        at org.jf.dexlib2.analysis.ClassPathResolver.<init>(ClassPathResolver.java:145)
        at org.jf.dexlib2.analysis.ClassPathResolver.<init>(ClassPathResolver.java:105)
        at org.jf.baksmali.AnalysisArguments.loadClassPathForDexFile(AnalysisArguments.java:129)
        at org.jf.baksmali.AnalysisArguments.loadClassPathForDexFile(AnalysisArguments.java:86)
        at org.jf.baksmali.DisassembleCommand.getOptions(DisassembleCommand.java:207)
        at org.jf.baksmali.DeodexCommand.getOptions(DeodexCommand.java:71)
        at org.jf.baksmali.DisassembleCommand.run(DisassembleCommand.java:181)
        at org.jf.baksmali.Main.main(Main.java:102)
Caused by: org.jf.dexlib2.analysis.ClassPathResolver$NotFoundException: Could not find classpath entry boot.oat
        at org.jf.dexlib2.analysis.ClassPathResolver.loadLocalOrDeviceBootClassPathEntry(ClassPathResolver.java:216)
        at org.jf.dexlib2.analysis.ClassPathResolver.<init>(ClassPathResolver.java:120)
        ... 7 more
```

把手机/system/framework/arm64/底下的文件全部拷贝到SystemUI.odex所在的目录底下：

![在这里插入图片描述](images/20190313122712544.png)