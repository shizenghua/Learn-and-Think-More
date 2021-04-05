# Unidbg模拟执行大厂so实操教程(一) 先把框架搭起来

## 一、目标

从这篇文章开始，我们一步一步介绍如何 填坑、调试并修改原工程 代码 **unidbg** 到跑出某电商Appsign的过程。

本文使用的原始代码在 https://github.com/zhkl0228/unidbg ,版本是 0.9.1

## 二、步骤

unidbg的作者更新的挺快，目前使用的是github上的最新的版本

参考 [Unidbg使用指南(一)](http://91fans.com.cn/post/unidbgone/),我们把基本框架搭起来

```java
 private static LibraryResolver createLibraryResolver() {
    return new AndroidResolver(23);
}

private static AndroidEmulator createARMEmulator() {
    return new AndroidARMEmulator("test");
}

private final AndroidEmulator emulator;
private final VM vm;
private Module module;
private DvmClass aBitmapkitUtils;

//初始化
public runliudq(){
    emulator = createARMEmulator();
    final Memory memory = emulator.getMemory();
    // 设置 sdk版本 23
    memory.setLibraryResolver(createLibraryResolver());

    vm = emulator.createDalvikVM(new File("/Users/fenfei/Desktop/V9.2.2.85371_T1_350271430_lc029.apk"));

      // vm.setDvmClassFactory(new ProxyClassFactory());
    vm.setJni(this);
    // 是否打印日志
      vm.setVerbose(true);
}

public void runJni(){
    DalvikModule dm = vm.loadLibrary("jdbitmapkit", false);

    dm.callJNI_OnLoad(emulator);
       module = dm.getModule();

}

@Override
public DvmObject<?> getStaticObjectField(BaseVM vm, DvmClass dvmClass, String signature) {
    switch (signature) {
        case "com/jingdong/common/utils/BitmapkitUtils->a:Landroid/app/Application;":
            return vm.resolveClass("android/app/Application", vm.resolveClass("android/content/ContextWrapper", vm.resolveClass("android/content/Context"))).newObject(null);
    }

    return super.getStaticObjectField( vm,  dvmClass,  signature);
}
```

这里需要解释的有以下几部分:

- emulator.createDalvikVM 创建虚拟机的时候，我们可以载入apk，这样内部实现了获取包名和包签名等jni操作
- 对于jni的操作，作者实现了两种，setJni和setDvmClassFactory，这里，我们先使用setJni的方式，并且继承了AbstractJni类
- 对于静态字段(成员变量)的操作，我们需要重写getStaticObjectField函数，通过对signature的解析来一一实现，这个感觉没有AndroidNativeEmu实现的优雅。

好了框架先搭到这里，明天继续……



# Unidbg模拟执行大厂so实操教程(二)

## 一、目标

从错误提示上看，是 **CallObjectMethod** 报错,看过之前AndroidNativeEmu的同学一定知道，这个还是 jmethodID相同 的梗。

## 二、步骤

我在github上也给作者提issues，但是没有找到很优雅的解决方案。只要用个临时方案来解决。

在 unidbg/unidbg-android/src/main/java/com/github/unidbg/linux/android/dvm/DalvikVM.java 的第47行增加

```java
if(name.equals("android/app/Activity"))
	name = "android/app/Application";
```

因为在这个so里面 **Activity** 类根本就没有实例化，可能纯粹为了为难模拟执行吧,所以就这么硬编码一下，不过用Unidbg跑其他so的时候要把这两行代码注释掉，避免产生bug。

运行一下看结果

```bash
java.lang.UnsupportedOperationException: android/app/Application->getPackageManager()Landroid/content/pm/PackageManager;
```

这次提示是没有找到 **Application→getPackageManager** 方法

上篇文章我们说了，jni操作，作者在AbstractJni类里面实现了，实际上 **getPackageManager** 作者已经实现了，只是他没有响应 Application对象。

我们在 AbstractJni.java 代码中的 callObjectMethod 函数里面修改，增加 Application 对象的响应

```java
case "android/app/Application->getPackageManager()Landroid/content/pm/PackageManager;":
case "android/content/Context->getPackageManager()Landroid/content/pm/PackageManager;":
	return vm.resolveClass("android/content/pm/PackageManager").newObject(null);
```

再跑，报错

```bash
java.lang.UnsupportedOperationException: android/app/Application->getPackageName()Ljava/lang/String;
```

道理一样还是在 AbstractJni.java 代码中的 callObjectMethod 函数里面修改，增加 Application 对象的响应

```java
case "android/app/Application->getPackageName()Ljava/lang/String;":
case "android/content/Context->getPackageName()Ljava/lang/String;": {
	String packageName = vm.getPackageName();
	if (packageName != null) {
		return new StringObject(vm, packageName);
	}
	break;

case "android/app/Application->getApplicationInfo()Landroid/content/pm/ApplicationInfo;":
case "android/content/Context->getApplicationInfo()Landroid/content/pm/ApplicationInfo;":
	return new ApplicationInfo(vm);
```

继续跑，这一次报了一个 android/content/pm/ApplicationInfo→sourceDir:Ljava/lang/String; 的错误，但是貌似可以不用修复。 然后成功的打印出了

```bash
Call [libjdbitmapkit.so]JNI_OnLoad finished, offset=120ms
```

### 开始调用

```java
aBitmapkitUtils = vm.resolveClass("com/jingdong/common/utils/BitmapkitUtils");
        DvmObject<?> strRc = aBitmapkitUtils.callStaticJniMethodObject(emulator,"getSignFromJni()(Landroid/content/Context;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;",
	vm.addLocalObject(null),
	vm.addLocalObject(new StringObject(vm,"asynInteface")),
	vm.addLocalObject(new StringObject(vm,"{\"intefaceType\":\"asynIntefaceType\",\"skuId\":\"100008667315\"}")),
	vm.addLocalObject(new StringObject(vm,"99001184062989-f460e22c02fa")),
	vm.addLocalObject(new StringObject(vm,"android")),
	vm.addLocalObject(new StringObject(vm,"9.2.2")));

System.out.println(strRc.getValue());
```

这次报错

```bash
java.lang.UnsupportedOperationException: java/lang/StringBuffer-><init>()V
```

我们来实现 StringBuffer ,Unidbg的好处是实现java类比较简单:

在 AbstractJni.java 代码中的 newObjectV 函数里面我们把 StringBuffer 和 Integer 都实现了：

```java
case "java/lang/StringBuffer-><init>()V":{
	return vm.resolveClass("java/lang/StringBuffer").newObject(new StringBuffer());
}
case "java/lang/Integer-><init>(I)V" :{
	return vm.resolveClass("java/lang/Integer").newObject(new Integer(vaList.getInt(0)));
}
```

这次报错

```bash
java.lang.UnsupportedOperationException: java/lang/StringBuffer->append(Ljava/lang/String;)Ljava/lang/StringBuffer;
```

我们需要在 AbstractJni.java 代码中的 callObjectMethodV 函数里实现 append和toString

```java
case "java/lang/Integer->toString()Ljava/lang/String;":{

    Integer iUse =  (Integer)dvmObject.getValue();
    return new StringObject(vm, Integer.toString(iUse));
}

case "java/lang/StringBuffer->toString()Ljava/lang/String;":{
    StringBuffer str = (StringBuffer) dvmObject.getValue();
    return new StringObject(vm,str.toString());
}
case "java/lang/StringBuffer->append(Ljava/lang/String;)Ljava/lang/StringBuffer;": {
    StringBuffer str = (StringBuffer) dvmObject.getValue();
    StringObject serviceName = vaList.getObject(0);
    assert serviceName != null;
    return vm.resolveClass("java/lang/StringBuffer").newObject(str.append(serviceName.value));
}
```

继续跑

```bash
st=1607417268979&sign=15db6c5b8076570b5db2407c308d282f&sv=111
```

这次成功了，老规矩，我们验算下,这里使用Unidbg自带的xHook

```java
IxHook xHook = XHookImpl.getInstance(emulator); // 加载xHook，支持Import hook，文档看https://github.com/iqiyi/xHook
xHook.register("libjdbitmapkit.so", "gettimeofday", new ReplaceCallback() { // hook libttEncrypt.so的导入函数strlen
    Pointer pointer1 = null;
    @Override
    public HookStatus onCall(Emulator<?> emulator, HookContext context, long originFunction) {
        pointer1 = context.getPointerArg(0);
        // String str = pointer.getString(0);
        System.out.println("gettimeofday");
        // context.push(str);
        return HookStatus.RET(emulator, originFunction);
    }
    @Override
    public void postCall(Emulator<?> emulator, HookContext context) {
        // Pointer pointer = context.getPointerArg(0);
        if(pointer1 != null){
            // 这里把 时间写死
            byte[] buf = {(byte)0x91,(byte)0x50,(byte)0xc4,(byte)0x5f,(byte)0x15,(byte)0x97,(byte)0x09,(byte)0x00};
            pointer1.write(0,buf,0,8);
        }
        // ByteBuffer tv_sec = pointer1.getByteBuffer(0,8);
        System.out.println("gettimeofday Ok");  //  + context.pop() ); //  + ", ret=" + context.getIntArg(0));
    }
}, true);
```

最后结果

```bash
st=1606701201628&sign=59039230dc2e1ea27a4f250d9ec81b8c&sv=111
```

和我们之前算的是一样的。

## 三、总结

Unidbg在模拟java类上有优势，感觉AndroidNativeEmu更像个优雅的Demo，而Unidbg还在一直更新，实用价值可能更大一点。