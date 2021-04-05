# Android 动态调试so文件

## 一、准备工作

- 首先使用工具获取到正确的动态链接库文件，一般来说有四种类型：
  - `arm64-v8a`：ARMv8，64位
  - `armeabi-v7a`：ARMv7，32位
  - `x86`：x86，32位
  - `x86_64`：x86_64，64位
- 大部分情况下，在真机上调试都使用ARM架构的so文件，可以根据设备是32位还是64位，选择对应的so文件进行调试。
- 其次我们要查看我们Android的编译类型，可以查看`ro.debuggable`的值：

```
getprop ro.debuggable
1
```

- 如果该值为1，则证明这个ROM本身是可调试的，同时我们也可以调试手机上任意的应用，一般来说很多基于AOSP编译的ROM都是这样，这样就可以直接进行调试。
- 如果是0，则必须APK本身是可调试的才行，也就是要检查`AndroidManifest.xml`中`android:debuggable`的值，这个值对于`debug`版本的APK默认值是`true`，对`release`版本的APK默认值是`false`。如果是`false`的话，那么想调试这个应用就只能重新编译它，或者找一台ROM可调试的手机来了。
- 同时你还需要有`adb`上的root权限，这样才可以进行调试。



## 二、设置IDA Pro调试参数

- 使用IDA打开so文件，对其进行反汇编，然后在右上方修改调试器为`Remote ARM Linux/Android debugger`。
- 然后设置调试服务器地址，选择`Debugger`菜单中的`Debugger Options`选项，在Hostname中填入`127.0.0.1`，端口保持默认的23946即可。

## 三、在目标机上启动调试服务器

- 在IDA的安装目录里，找到`dbgsrv`文件夹（Mac OS上是一个符号链接），下面有用于调试的服务器端软件，我们使用`android_server`，这是配合32位IDA使用的调试服务端，如果使用64位的就选择`android_server64`，我们将`android_server`传输到我们的Android手机上的`/data/local/tmp`路径下。

```
adb push android_server /data/local/tmp/
```

- 注意：因为太多的教程都使用了`/data/local/tmp/`这个目录来保存调试服务器文件，以至于很多反调试手段加入了对此路径的检测。如果发现有反调试机制限制（下面会讲到），可以放入其他路径。
- 然后在使用adb命令进入Android的Shell，取得其root权限，切换到`android_server`的存放目录：

```
adb root
adb shell
# cd /data/local/tmp
```

- 使用`chmod`命令为此文件赋予执行权限，然后启动，默认`android_server`监听的是23946端口：

```
/data/local/tmp # ls
android_server
/data/local/tmp # chmod +x android_server
/data/local/tmp # ./android_server
IDA Android 32-bit remote debug server(ST) v1.22. Hex-Rays (c) 2004-2017
Listening on 0.0.0.0:23946...
```

## 四、设置端口转发

- 这时候`android_server`已经在监听Android设备的23946端口，我们还需要将这个端口转发到我们的电脑上：

```
adb forward tcp:23946 tcp:23946
```

## 五、以调试模式启动Activity

- 现在，我们可以开始着手调试程序了，我们先以调试模式启动程序的主Activity，命令格式为：

```
adb shell am start -D -n 包名/类名
```

- 其中`-D`代表以调试模式启动，对于我们的测试程序，包名是`com.wrlus.reversedemo`，Activity类名是`com.wrlus.reversedemo.MainActivity`，所以就输入：

```
adb shell am start -D -n com.wrlus.reversedemo/.MainActivity
Starting: Intent { cmp=com.wrlus.reversedemo/.MainActivity }
```

- 这时候手机会启动Activity，并显示`Waiting for Debugger`的界面。

## 六、开始单步调试

- 启动程序后就可以开始调试了，我们找到IDA的`Debugger`菜单，选择`Attach process`，在弹出的窗口中选择我们的被调试进程`com.wrlus.reversedemo`，当IDA进入调试状态后，就已经成功附加到目标进程。
- IDA会首先断在程序刚开始执行的地方，这时候可以选择继续运行让程序继续向下执行。然后就可以在手机上操作了。一般来说下断点直接在调试前的汇编中下断点即可，在执行到断点时候也可以以伪代码的形式调试。

## 七、反调试

- 以上方法针对的是一般的动态库文件，但是对于有些APK，这样附加之后会直接闪退，那么就是做了反调试了。上面方法附加动态库的时候，是执行过`.init_array`和`JNI_OnLoad()`的，一般来说反调试都是在这些方法中做的。所以要想绕过反调试，必须在这两个执行之前就下好断点。
- 针对以上，我们改变一下调试的方法。

## 八、更改IDA Pro调试参数

- 要想在`.init_array`和`JNI_OnLoad()`之前就中断，就需要在加载库的一开始就中断。所以我们需要修改IDA Pro的调试附加参数，选择`Debugger`菜单中的`Debugger Options`选项，勾选`Suspend on thread start/exit`和`Suspend on library load/unload`，使得在进程开始/结束和动态库加载/卸载的时候，自动进行中断。

## 九、使用DDMS

- 完成后我们依旧是以调试模式启动Activity

```
> adb shell am start -D -n com.wrlus.reversedemo/.MainActivity
Starting: Intent { cmp=com.wrlus.reversedemo/.MainActivity }
```

- 这时候手机会启动Activity，并显示`Waiting for Debugger`的界面。然后再使用IDA Pro正常进行`Attach process`，这时候IDA Pro会加载库文件并中断。这时候先不要急着继续运行，我们需要另一个工具——`DDMS`。
- `DDMS`是Android SDK提供的工具，在Android Studio 3.0以上，就不可以通过AS来启动了，但我们可以在SDK的目录下找到他它，位于SDK目录的`tools`文件夹下，双击`monitor.bat`就可以启动它。
- 启动之后我们需要找到待调试程序的端口，待调试程序左侧会有个debug小虫子的图标，第二列数字就是端口，比如这里我的端口为8638。

## 十、使用JDB附加目标程序

- 为了使程序恢复运行，我们使用jdb附加目标程序，jdb是JDK中提供的工具，我们使用以下命令：

```
jdb -connect com.sun.jdi.SocketAttach:hostname=127.0.0.1,port=8700
设置未捕获的java.lang.Throwable
设置延迟的未捕获的java.lang.Throwable
正在初始化jdb...
>
```

- 其中8638就是上一步找到的端口号，这时候你会发现程序的`waiting for debugger`消失，程序已经恢复运行。

## 十一、寻找目标函数

- 由于设置了IDA Pro的中断参数，所以在每个线程启动的时候都会进行中断，可以通过查看IDA控制台来查看现在装载的是哪一个so文件。如果不是我们关心的，可以直接点击绿色箭头继续运行。
- 一般来说，到我们的so的时候IDA Pro会出现一个提示，询问你现在正加载的so文件（位于`/data/data/{package_name}/lib`中）和你正查看的so文件（位于你的电脑中）是否是同一个文件，这时候我们就选择`Yes`即可。
- 选择之后IDA Pro会中断在`linker.so`中，这是加载so文件所必须的一个库文件。现在我们可以按下`Ctrl+S`快捷键，查看当前装载的模块，找到我们想要调试的so文件并双击，就可以跳转到这个so文件的起始地址。如果有多个同名文件，只需要选择带有执行权限（X）的那个。
- 这时候我们再打开另一个IDA Pro，用来查看目标函数（当然我们最关心的是`.init_array`和`JNI_OnLoad()`）的偏移地址，然后再和当前so文件的起始地址相加，就可以得到实际的函数地址。实际地址=起始地址+偏移地址，由于现在的Android系统都实现了ASLR，所以这个起始地址每次都是不同的。
- 找到目标函数之后，就可以下断点进行调试了。记得绕过反调试哦。

## 十二、常见问题

1. IDA Pro提示`Please be carrful, the debug path looks odd! 'xxxxx.so' Do you really want IDA to access this path (possibly a remote server)?`

- 直接点Yes就好，这个so文件一般不是你要关系的那个。

1. IDA Pro提示`got SIGSEGV signal ...`，然后程序终止？

- 通常是由于反调试机制终止了程序，你需要按照上面反调试的方法操作，并且找到反调试的代码并将其绕过。当然也可能是程序本身崩溃。

