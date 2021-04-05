# Xposed项目搭建

## 项目结构

[![img](https://ws4.sinaimg.cn/large/006tKfTcly1fo4gkfn5i3j30bv0mqmz4.jpg)](https://ws4.sinaimg.cn/large/006tKfTcly1fo4gkfn5i3j30bv0mqmz4.jpg)

## 新建一个lib文件夹，然后将api-82.jar复制进去

新建一个lib文件夹，然后将api-82.jar复制进去
[下载链接](https://jcenter.bintray.com/de/robv/android/xposed/api/)
下载如下两个文件：[api-82-sources.jar](https://bintray.com/rovo89/de.robv.android.xposed/download_file?file_path=de%2Frobv%2Fandroid%2Fxposed%2Fapi%2F82%2Fapi-82-sources.jar)和[api-82.jar](https://bintray.com/rovo89/de.robv.android.xposed/download_file?file_path=de%2Frobv%2Fandroid%2Fxposed%2Fapi%2F82%2Fapi-82.jar)

## 在app的build.gradle中将添加如下语句

```
provided files('lib/api-82.jar')
```

[![img](https://ws4.sinaimg.cn/large/006tKfTcly1fo4h1qcqnej30nw0gbjuc.jpg)](https://ws4.sinaimg.cn/large/006tKfTcly1fo4h1qcqnej30nw0gbjuc.jpg)
作用：将libs中的Xposed框架API引用到项目中（构建依赖）

## 在AndroidManifest.xml将自己标识为一个Xposed模块，语句添加在如下位置

```
<meta-data
    android:name="xposedmodule"
    android:value="true" />
<meta-data
    android:name="xposeddescription"
    android:value="我是一个Xposed例程" />
<meta-data
    android:name="xposedminversion"
    android:value="30" />
```

[![img](https://ws4.sinaimg.cn/large/006tKfTcly1fo4glg5gp9j30gu0estbb.jpg)](https://ws4.sinaimg.cn/large/006tKfTcly1fo4glg5gp9j30gu0estbb.jpg)
作用：

- xposedmodule：value为true，表示自己是一个xposed模块
- xposeddescription：value中的文字就是对模块的描述，这些能够在手机上的Xposed框架中看到
- xposedminversion：xposed最低版本

## 在类里编写hook代码

[![img](https://ws3.sinaimg.cn/large/006tKfTcly1fo4el2kqptj30sc0d475m.jpg)](https://ws3.sinaimg.cn/large/006tKfTcly1fo4el2kqptj30sc0d475m.jpg)
这里我新建了一个HookToast类，但是你可以在MainActivity里写，只要在后面写好xposed模块的入口点就好了。
之前在AndroidManifest.xml中标识了我们的项目是一个Xposed模块，可是我们可能会有许多Activity，
它怎么才能知道模块的入口在哪呢？
所以，下面要告诉Xposed框架，我们的应用中，Xposed模块的入口到底在哪。
具体代码不给出，我只是记录一下，应该怎么编辑项目而已。

## 标注Xposed模块入口

右键点击 main ， 选择new –> Folder –>Assets Folder，然后确认即可。
在assets中new一个file，文件名为xposed_init（文件类型选text），并在其中写上入口类的完整路径（下面是我的类路径，你们填自己的，就是activity中packege后面的包名）
[![img](https://ws3.sinaimg.cn/large/006tKfTcly1fo4eok5zihj31g00w0q96.jpg)](https://ws3.sinaimg.cn/large/006tKfTcly1fo4eok5zihj31g00w0q96.jpg)
这样，xposed框架就能够读取xposed_init中的信息来找到模块的入口。

## Run

[![img](https://ws1.sinaimg.cn/large/006tKfTcly1fo4h18eaarj30s70ikq62.jpg)](https://ws1.sinaimg.cn/large/006tKfTcly1fo4h18eaarj30s70ikq62.jpg)
请确保禁用Instant Run（File -> Settings -> Build, Execution, Deployment -> Instant Run），否则您的类不会直接包含在APK中，导致HOOK失败。
[![img](https://ws3.sinaimg.cn/large/006tKfTcly1fo4h4j6ub3j30ju02waav.jpg)](https://ws3.sinaimg.cn/large/006tKfTcly1fo4h4j6ub3j30ju02waav.jpg)

[# xposed](https://eternalsakura13.com/tags/xposed/)