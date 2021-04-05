# 某电商App签名算法解析(二)

## 一、目标

[某电商App签名算法解析(一)](http://91fans.com.cn/post/jdsignone/)的文章里面我们分析了签名算法的位置，今天我们和之前的 [某右协议分析(四)](http://91fans.com.cn/post/zysignfour/) 文章一样，利用Xposed + NanoHTTPD 来实现rpc调用，做签名服务

###### Note:

- Xposed rpc调用
- NanoHTTPD
- 某电商App 9.2.2

## 二、分析

签名函数一共5个参数，

![jdsigntwo1](images/images/jdsigntwo1.png)1:jdsigntwo1

通过之前的frida的分析结果

- str = functionId
- str2 = body
- str3 = uuid
- str4 = client
- str5 = clentVersion

这里面就是第一个参数 Context 比较麻烦，需要我们构造一个，这里我们hook android.content.ContextWrapper来获取Context

```java
try  {
	Class<?>  ContextClass  =   XposedHelpers.findClass("android.content.ContextWrapper",  loadPackageParam.classLoader);
    XposedHelpers.findAndHookMethod(ContextClass,  "getApplicationContext",  new  XC_MethodHook()  {
    @Override
    protected  void  afterHookedMethod(MethodHookParam  param)  throws  Throwable  {
		super.afterHookedMethod(param);
        if  (applicationContext  !=  null)
			return;
            applicationContext  =  (Context)  param.getResult();
            log("-->得到上下文");
	    }
    });
}  catch  (Throwable  t)  {
	log("-->获取上下文出错");
}
```

参数都ok了，就可以调用签名函数了

```java
Class<?> clazzJDUtils = null;
try {
	clazzJDUtils = loadPackageParam.classLoader.loadClass("com.jingdong.common.utils.BitmapkitUtils");
    log("load class:" + clazzJDUtils);
} catch (Exception e) {
	log("load class err:" + Log.getStackTraceString(e));
    return newFixedLengthResponse("BitmapkitUtils load class is null");
}

...

String rc = (String) XposedHelpers.callStaticMethod(clazzJDUtils, "getSignFromJni", applicationContext ,str,str2,str3,str4,str5);
log("getSignFromJni = "+rc);
```

结果正常返回了签名，把这个签名加到url后面就可以正常返回结果。

```bash
getSignFromJni = st=1605755406793&sign=43f8086393e3e9fefbd5d6ae87fcd8ce&sv=110
```

我们拿go语言写个请求程序，搜索一下 “柚子”

![jdsend](images/images/jdsend.png)1:sendrc

结果正常， 收工。

###### Tip:

本文的目的只有一个就是学习更多的逆向技巧和思路，如果有人利用本文技术去进行非法商业获取利益带来的法律责任都是操作者自己承担，和本文以及作者没关系，本文涉及到的代码项目可以去 奋飞的朋友们 知识星球自取，欢迎加入知识星球一起学习探讨技术。有问题可以加我wx: fenfei331 讨论下。