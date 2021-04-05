# Magisk学习之刷入vbmeta.img及关闭avb2.0校验

最近刷了很多机型的Magisk，经历了N次变砖和修复，也有了一些经验，Magisk提供了三种方式刷入Magisk，分别是：

1. 具有root权限情况下直接manager中直接安装
2. 在能获取到boot.img的情况下，patch boot.img然后fastboot刷入
3. 在三方recovery中直接刷入magisk.zip

这三种安装方式教程很多，主要介绍一下直接安装或者patch boot.img刷完magisk变砖之后遇到的分区验证导致变砖问题以及如何解决。

 

从刷入Magisk的方式都需要操作boot.img也可以看出来，magisk的核心原理就是修改和替换了负责安卓系统启动的boot.img中的Ramdisk部分，该映像包括了一系列初始化init进程和启动时使用的rc文件。

 

如果直接能拿到Root权限，可以考虑直接安装或者选择并修补boot.img的方式刷入，两种方式的本质相同，均是获取到boot.img后进行patch，只不过实现方式不同，一种依赖Fastboot进行刷入，一种则是Magisk Manager中进行刷入。

 

获取boot.img的方式有两种：

1. 从官方的ROM中提取，不同ROM的打包方式不同，例如一加是payload.bin，有专门的脚本提取，而google原生就是一个zip包解压即可，再例如三星可能是AP开头的tar文件。

2. 如果有root权限，直接从设备中提取，/dev/block/by-name下根据软连接找到boot.img，然后使用dd命令将img写出。如果是A/B分区，一般采用boot_a.img,但为了防止变砖后无法还原，建议用这种方式将recovery.img boot.img vbmeta.img都备份一份。

   ```
   ls -l /dev/block/by-name/ | grep boot
   dd if=/dev/block/sdc6 of=/sdcard/boot.img
   ```

如果在刷入Magisk时变砖无法开机或着无限重启，通常只需在fastboot中将正常的boot.img重新刷回即可，也是为什么要提前备份boot.img等的原因。

# avb2.0验证导致手机无法启动

在部分机型中，可能由于vbmeta.img的验证导致设备无法启动，验证启动（Verified Boot）是Android一个重要的安全功能，主要是为了访问启动镜像被篡改，提高系统的抗攻击能力，简单描述做法就是在启动过程中增加一条校验链，即 ROM code 校验 BootLoader，确保 BootLoader 的合法性和完整性，BootLoader 则需要校验 boot image，确保 Kernel 启动所需 image 的合法性和完整性，而 Kernel 则负责校验 System 分区和 vendor 分区。

 

由于 ROM code 和 BootLoader 通常都是由设备厂商 OEM 提供，而各家实际做法和研发能力不尽相同，为了让设备厂商更方便的引入 Verified boot 功能，Google 在 Android O上推出了一个统一的验证启动框架 Android verified boot 2.0，好处是既保证了基于该框架开发的verified boot 功能能够满足 CDD 要求，也保留了各家 OEM 定制启动校验流程的弹性。

## Fastboot解决方案

简单的说，就是部分厂商机型可能由于avb2.0验证boot.img是否被修改，导致刷入magisk或者三方Recovery后陷入假变砖无限重启的情况，此时将备份的vbmeta.img重新刷入并关闭验证即可：

 

`fastboot --disable-verity --disable-verification flash vbmeta vbmeta.img`

 

这种方式简单易用，适合个人玩家，但如果是用在生产中，成千台设备需要批量刷入magisk时，这种方式太过人工，因此还可以通过定制Magisk代码的方式来一键刷入Magisk。

## Magisk源代码定制解决方案

来看一下Fastboot的原理，查阅[上一步中FastBoot的源码](https://android.googlesource.com/platform/system/core/+/master/fastboot/fastboot.cpp)可以看到：

```
// There's a 32-bit big endian |flags| field at offset 120 where
    // bit 0 corresponds to disable-verity and bit 1 corresponds to
    // disable-verification.
    //
    // See external/avb/libavb/avb_vbmeta_image.h for the layout of
    // the VBMeta struct.
    uint64_t flags_offset = 123 + vbmeta_offset;
    if (g_disable_verity) {
        data[flags_offset] |= 0x01;
    }
    if (g_disable_verification) {
        data[flags_offset] |= 0x02;
    }
```

实际上只是对vbmeta.img中120偏移处进行了两个bit位的处理，即vbmeta.img中是否关闭验证有两个flag可以控制，我们只需依葫芦画瓢:

1. 从手机提取原始vbmeta.img（参考Magisk直接安装时寻找boot.img的操作）
2. 修改120偏移处的flag (参考Magisk对三星AP文件中的vbmeta.img修复操作)
3. 重新刷回系统中

### Magisk如何在非Fastboot下刷入boot.img

这三步中比较困难的就是第三步，将patch完的vbmeta.img重新刷回系统中，因此这里参考了Magisk在有root权限时patch完boot.img之后直接刷入new_boot.img的操作。

```
// MagiskInstaller.kt 直接安装的入口
protected fun direct() = findImage() && extractZip() && patchBoot() && flashBoot()
 
private fun flashBoot(): Boolean {
        if (!"direct_install $installDir $srcBoot".sh().isSuccess)
            return false
        "run_migrations".sh()
        return true
    }
 
// shell脚本
direct_install() {
  rm -rf $MAGISKBIN/* 2>/dev/null
  mkdir -p $MAGISKBIN 2>/dev/null
  chmod 700 $NVBASE
  cp -af $1/. $MAGISKBIN
  rm -f $MAGISKBIN/new-boot.img
  echo "- Flashing new boot image"
  flash_image $1/new-boot.img $2
  if [ $? -ne 0 ]; then
    echo "! Insufficient partition size"
    return 1
  fi
  rm -rf $1
  return 0
}
 
# $1参数是patch后的new-boot.img路径，$2参数是设备系统中boot.img路径
flash_image() {
  # Make sure all blocks are writable
  $MAGISKBIN/magisk --unlock-blocks 2>/dev/null
  case "$1" in
    *.gz) CMD1="$MAGISKBIN/magiskboot decompress '$1' - 2>/dev/null";;
    *)    CMD1="cat '$1'";;
  esac
  if $BOOTSIGNED; then
    CMD2="$BOOTSIGNER -sign"
    ui_print "- Sign image with verity keys"
  else
    CMD2="cat -"
  fi
  if [ -b "$2" ]; then
    local img_sz=`stat -c '%s' "$1"`
    local blk_sz=`blockdev --getsize64 "$2"`
    [ $img_sz -gt $blk_sz ] && return 1
    eval $CMD1 | eval $CMD2 | cat - /dev/zero > "$2" 2>/dev/null
  elif [ -c "$2" ]; then
    flash_eraseall "$2" >&2
    eval $CMD1 | eval $CMD2 | nandwrite -p "$2" - >&2
  else
    ui_print "- Not block or char device, storing image"
    eval $CMD1 | eval $CMD2 > "$2" 2>/dev/null
  fi
  return 0
}
```

看起来很复杂，核心写入的步骤其实就是eval $CMD1 | eval $CMD2 | cat - /dev/zero > "$2" 2>/dev/null，通过这行命令就可以实现不在Fastboot中通过shell命令完成boot.img和vbmeta.img的写入（也许也可以类推到其他img映像分区）

### 实现vbmeta.img的重写

而且vbmeta.img相比于boot.img比较简单，不需要签名、验证等步骤，因此可以简单提炼为:

```
# 先找到vbmeta.img
vbmeta_block_name=`find /dev/block \( -type b -o -type c -o -type l \) -iname vbmeta | head -n 1`
vbmeta_block=`readlink -f $vbmeta_block_name`
# 将patch好的vbmeta.img写入
eval 'cat '/sdcard/patched_vbmeta.img'' | eval 'cat -' | cat - /dev/zero > $vbmeta_block 2>/dev/null
```

再加上patch操作，完整代码为：

```
fun disableVbMetaImg() {
        try {
            // find vbmeta.img block
            val vbmetaPath = ShellUtils.fastCmd("readlink -f `find /dev/block \\( -type b -o -type c -o -type l \\) -iname vbmeta | head -n 1`")
            if (TextUtils.isEmpty(vbmetaPath)) {
                Log.w(Const.TAG, "could not find vbmeta.img!")
                return
            }
            val patchedVbmetaPath = "/sdcard/patched_vbmeta.img"
            if (!Shell.sh("dd if=$vbmetaPath of=$patchedVbmetaPath").exec().isSuccess) {
                Log.w(Const.TAG, "extract vbmeta.img failed!")
                return
            }
            val patchedVbMeta = SuFile.open(patchedVbmetaPath).readBytes()
            // There's a 32-bit big endian |flags| field at offset 120 where
            // bit 0 corresponds to disable-verity and bit 1 corresponds to
            // disable-verification.
            ByteBuffer.wrap(patchedVbMeta).putInt(120, 2)
            File(patchedVbmetaPath).writeBytes(patchedVbMeta)
            // rewrite vbmeta.img
            Log.d(Const.TAG, "rewrite vbmeta.img finished: $patchedVbmetaPath to $vbmetaPath")
            Shell.sh("eval 'cat '$patchedVbmetaPath'' | eval 'cat -' | cat - /dev/zero > $vbmetaPath 2>/dev/null").exec()
        } catch (e: Throwable) {
            Log.i(Const.TAG, "disable vb_meta.img avb2.0 error:", e)
        }
    }
```