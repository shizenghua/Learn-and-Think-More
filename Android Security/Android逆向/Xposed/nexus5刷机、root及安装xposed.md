# nexus5刷机、root及安装xposed

## nexus5刷机

`adb reboot bootloader`
`fastboot oem unlock`
在这里根据不同的版本下载镜像：https://developers.google.com/android/images
nexus5是hammerhead

然后运行flash-all.sh脚本即可刷机

## root

### 下载root工具包

https://download.chainfire.eu/363/CF-Root/CF-Auto-Root/CF-Auto-Root-hammerhead-hammerhead-nexus5.zip

### 运行root 脚本

`adb reboot bootloader`
命令行进入解压下载的工具包中`chmod +x root-mac.sh`
`./root-mac.sh`
手机出现红色android,等待重启,查看是否已安装SuperSu权限管理工具

## 安装xposed（Android 4.0.3 up to Android 4.4)

**关于xposed，有两种安装方法，对于低版本，直接安装xposed install即可，高版本需要刷入twrp（见下）**
http://repo.xposed.info/module/de.robv.android.xposed.installer
`adb install xx.apk`安装即可

## 安装xposed (Android 5.0.1，更高版本类似)

## 下载twrp

下载你的设备对应的[twrp](https://twrp.me/Devices/),比如选择的是Google Nexus5，twrp-3.0.2-0-hammerhead.img

- 进入bootloader刷机界面
  `adb reboot bootloader`

- 输入以下指令fastboot devices判断设备是否连接

- 刷入TWRP

  ```
  fastboot flash recovery twrp-3.0.2-0-hammerhead.img
  fastboot reboot
  ```

- 进入刷机界面

  ```
  adb reboot recovery
  ```

  ## 下载xposed框架和应用

  ### 下载安装包

  官方下载地址

  Xposed for Lollipop and Marshmallow

  根据设备CPU型号下载对应的框架和应用,比如如果是nexus5 android5.0.1，选择

  xposed-v86-sdk21-arm.zip

  和

  XposedInstaller_3.1.apk

### 将xposed-v86-sdk21-arm.zip push到手机中

`adb push xposed-v86-sdk21-arm.zip /sdcard`
在twrp点击install ,选中xposed-v86-sdk21-arm.zip,然后点击flash,完成后重启

### 安装XposedInstaller_3.1.apk

`adb install XposedInstaller_3.1.apk`
安装完成后打开应用查看是否已激活

## 提供其他的一些参考

http://bbs.gfan.com/android-7599937-1-1.html
http://bbs.gfan.com/android-7537408-1-1.html