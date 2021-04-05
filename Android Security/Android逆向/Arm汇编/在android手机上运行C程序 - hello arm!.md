# 在android手机上运行C程序 - hello arm!

# gcc交叉编译

暂时不想研究[Android.mk](https://developer.android.com/ndk/guides/android_mk.html?hl=zh-cn)
就取巧了一下，用的[独立工具链](https://developer.android.com/ndk/guides/standalone_toolchain.html?hl=zh-cn#itc)
**然而不行**

## 添加环境变量

打开~/.bashrc文件，填入这些内容。

```
#android-ndk-gcc
export NDK=~/Library/Android/sdk/ndk-bundle
export SYSROOT=$NDK/platforms/android-21/arch-arm
export CC="$NDK/toolchains/arm-linux-androideabi-4.9/prebuilt/ \
linux-x86/bin/arm-linux-androideabi-gcc-4.9 --sysroot=$SYSROOT"
```

`source ~/.bashrc`启用。
[![img](https://ws1.sinaimg.cn/large/006tKfTcly1fo2gnd3pjlj30fw02wwf1.jpg)](https://ws1.sinaimg.cn/large/006tKfTcly1fo2gnd3pjlj30fw02wwf1.jpg)

## 编写程序

```
#include<stdio.h>
int main(int argc, char* argv[])
{
        printf("hello arm!\n");
        return 0;
}
```

`CC -o hello-arm -c hello-arm.c`
理论上这样就可以了，然而我这还是不可以。无法解决的报错。
[![img](https://ws4.sinaimg.cn/large/006tKfTcly1fo2hiabc45j30ac012aa0.jpg)](https://ws4.sinaimg.cn/large/006tKfTcly1fo2hiabc45j30ac012aa0.jpg)

# 使用ndk-build

因为上面那种方法GG,就只能用这种方法了~

## 添加环境变量

打开~/.bashrc
输入下面这两句
[![img](https://ws2.sinaimg.cn/large/006tKfTcly1fo2iuyulkrj30ng0b0tb2.jpg)](https://ws2.sinaimg.cn/large/006tKfTcly1fo2iuyulkrj30ng0b0tb2.jpg)

```
export NDK=~/Library/Android/sdk/ndk-bundle
alias ndk-build="$NDK/ndk-build"
```

然后`source ~/.bashrc`，就可以使用ndk-build了。

## 新建android工程和jni目录

这个肯定不用我说了……就建一个empty project即可
[![img](https://ws4.sinaimg.cn/large/006tKfTcly1fo2ixb4c11j30rt0ihmzt.jpg)](https://ws4.sinaimg.cn/large/006tKfTcly1fo2ixb4c11j30rt0ihmzt.jpg)

## 编写c文件和Android.mk

新建脚本文件名为Android.mk,这是ndk-build需要的工程编译脚本,描述了编译程序的各种选项和依赖.
内容如下:

```
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_ARM_MODE := arm
LOCAL_MODULE := hello-arm
LOCAL_SRC_FILES := hello-arm.c
include $(BUILD_EXECUTABLE)
```

其中:
LOCAL_PATH表示本工程源码的路径, my-dir表示Android.mk的路径;
CLEAR_VARS让编译器清除已经定义过的宏,避免在编译多个模块时发生错误,因为这些宏是全局的,必须重新设置;
LOCAL_ARM_MODE指定程序使用的ARM指令模式;
LOCAL_MODULE指定生成的模块名,如果生成的是共享库,模块名会变为libhello-arm.so;
LOCAL_SRC_FILES指定源文件列表;
BUILD_EXECUTABLE表示生成的文件是可执行的,其他选项有BUILD_SHARED_LIBRARY(生成动态库),BUILD_STATIC_LIBRARY(生成静态库).

## 编译

在jni目录下，输入ndk-build，编译完成后的文件在libs/armeabi目录下.
[![img](https://ws4.sinaimg.cn/large/006tKfTcly1fo2izqzjhuj30fs09ymz6.jpg)](https://ws4.sinaimg.cn/large/006tKfTcly1fo2izqzjhuj30fs09ymz6.jpg)

## 执行

执行adb push hello-arm /data/local/tmp到手机上