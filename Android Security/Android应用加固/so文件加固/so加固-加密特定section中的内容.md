# so加固-加密特定section中的内容

url：https://www.jianshu.com/p/b4962a2c9584

本文参考自：[Android逆向之旅—基于对so中的section加密技术实现so加固](http://www.wjdiankong.cn/android逆向之旅-基于对so中的section加密技术实现so加固/)，增加了自己的实践过程，以及一些额外的验证和解释。

本文代码参见：https://github.com/difcareer/SoEncrypt

[Android逆向之旅—基于对so中的section加密技术实现so加固](http://www.wjdiankong.cn/android逆向之旅-基于对so中的section加密技术实现so加固/) 这篇文章写得真心好，建议先阅读一下原著，这里只是自己的实践过程（纸上得来终觉浅，绝知此事要躬行），和一些更细节的解释罢了。

## 一. 拆分section

这个demo的目的是为了将native函数getString()给保护起来（实际应用场景就是自己业务中的核心代码）。为了保护getString()，用到了[gcc的Attributes特性](https://gcc.gnu.org/onlinedocs/gcc-3.3/gcc/Variable-Attributes.html)：



```bash
__attribute__((section ("xxx")))
```

上述的文档中提到，给变量或者方法增加这个修饰后，编译器将把对应的代码或者数据放到你指定的section中。
 我们的demo中做了示例：



```java
__attribute__((section (".encrypt"))) jstring getString(JNIEnv* env) {
    static const char* txt __attribute__((section (".encrypt2"))) = "Str from native";
    return (*env)->NewStringUTF(env, txt);
};
```

我们同时给getString()函数和txt变量添加了这个属性，分别指定了不同名称。我们看一下编译后的so:

![img](https:////upload-images.jianshu.io/upload_images/1784193-b7c408ca3759946a.png?imageMogr2/auto-orient/strip|imageView2/2/w/951/format/webp)

Paste_Image.png

![img](https:////upload-images.jianshu.io/upload_images/1784193-b03d0a733d66ef16.png?imageMogr2/auto-orient/strip|imageView2/2/w/1038/format/webp)

Paste_Image.png

![img](https:////upload-images.jianshu.io/upload_images/1784193-c48decae9cd900aa.png?imageMogr2/auto-orient/strip|imageView2/2/w/883/format/webp)

Paste_Image.png

可以看到新增了我们自定义的section: encrypt、encrypt2，encrypt中存放getString()的代码，因此被映射为可执行，encrypt2中存放txt的数据，因此被映射为可写。

ok，到这里我们已经找到拆分核心代码到单独section的方法了。后续我们只要对这些section做加密即可保护核心代码。

## 二. 寻找解密时机

假设我们已经加密了这些section，运行的时候总是需要解密还原的，什么时机解密最好呢，当然是越早越好，最早可以在load so之后，执行JNI_Onload之前，这里也是需要gcc的[另外一个Attributes特性](https://gcc.gnu.org/onlinedocs/gcc/Common-Function-Attributes.html#Common-Function-Attributes)：



```tsx
__attribute__((constructor (n)))
```

文档中指出，constructor (priority)可以在后面指定一个优先级，数字越小，优先级越高，越先被执行。

关于这点我们在demo中也做了验证：



```tsx
void init_1() __attribute__((constructor (3)));
void init_getString() __attribute__((constructor (2)));
void init_2() __attribute__((constructor (1)));
```

我们申明了3个函数，优先级从低到高，按照规则，执行顺序应该是：init_2、init_getString、init_1，我们看一下so：

![img](https:////upload-images.jianshu.io/upload_images/1784193-4d52cf2b597506d6.png?imageMogr2/auto-orient/strip|imageView2/2/w/625/format/webp)

Paste_Image.png



已经按照优先级调整好了顺序。

我们看下日志：

![img](https:////upload-images.jianshu.io/upload_images/1784193-8376e2521dbd54c2.png?imageMogr2/auto-orient/strip|imageView2/2/w/306/format/webp)

Paste_Image.png



的确也是这个顺序。
 ok，这样我们就可以在这个特性的修饰下，尽早能做解密逻辑了。

## 三. 加密逻辑

先说一下加密，作者的加密算法很简单：字节取反。在misc/encrpt.c中，我们可以发现其核心逻辑是寻找叫做 encrypt 的 section，然后字节取反写回，同时计算将一些值计算了写入ehdr.e_entry（这个对于正常的so是0值）和ehdr.e_shoff（这个是section表的偏移量，修改这个值将导致找不到section，后面会看到加密效果），这些值在解密的时候需要。
 demo的misc下有编译后的脚本encrpt，需要在linux环境下执行，libencrypt.so是没有加密前的so，libencrypt2.so是加密后的so。你可以自行使用beyond compare比较差异。

## 四. 解密逻辑

回到最重要的解密逻辑了，我们在`__attribute__((constructor (n)))`修饰的方法`init_getString()`中实现了解密逻辑，其原理是，通过读取/proc/pid/maps中的内容，找到so被映射到内存中的地址，然后通过ehdr.e_entry和ehdr.e_shoff中的内容还原出decrypt section 的地址，字节取反恢复，内存写回。这样就做到了动态解密了。

## 五. 加密效果

使用ida打开misc/libencrypt2.so

![img](https:////upload-images.jianshu.io/upload_images/1784193-d64f205e660bbbfa.png?imageMogr2/auto-orient/strip|imageView2/2/w/438/format/webp)

Paste_Image.png



提示这个是因为修改了ehdr.e_shoff，破坏了第一个section 类型为SHT_NULL的规则。

![img](https:////upload-images.jianshu.io/upload_images/1784193-8ddd11b14f04a253.png?imageMogr2/auto-orient/strip|imageView2/2/w/599/format/webp)

Paste_Image.png



下一步直接解析section 报错。

![img](https:////upload-images.jianshu.io/upload_images/1784193-98bc3a4a267afdd6.png?imageMogr2/auto-orient/strip|imageView2/2/w/945/format/webp)

Paste_Image.png



正常的section都看不到了，看到的都是program sections（上一个图的提示）。

而apk可以正常运行（加密后的apk为misc/signed.apk）：

![img](https:////upload-images.jianshu.io/upload_images/1784193-56ba42f9219ec465.png?imageMogr2/auto-orient/strip|imageView2/2/w/638/format/webp)

Paste_Image.png

参考链接：
 [http://www.wjdiankong.cn/android%e9%80%86%e5%90%91%e4%b9%8b%e6%97%85-%e5%9f%ba%e4%ba%8e%e5%af%b9so%e4%b8%ad%e7%9a%84section%e5%8a%a0%e5%af%86%e6%8a%80%e6%9c%af%e5%ae%9e%e7%8e%b0so%e5%8a%a0%e5%9b%ba/](http://www.wjdiankong.cn/android逆向之旅-基于对so中的section加密技术实现so加固/)
 https://gcc.gnu.org/onlinedocs/gcc-3.3/gcc/Variable-Attributes.html
 https://gcc.gnu.org/onlinedocs/gcc/Common-Function-Attributes.html#Common-Function-Attributes



作者：difcareer
链接：https://www.jianshu.com/p/b4962a2c9584
来源：简书
著作权归作者所有。商业转载请联系作者获得授权，非商业转载请注明出处。