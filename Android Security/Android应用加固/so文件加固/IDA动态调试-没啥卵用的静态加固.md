# IDA动态调试-没啥卵用的静态加固

url：https://cloud.tencent.com/developer/article/1193475

前几天学习了so加固相关的知识[so加固-加密特定section中的内容](https://www.jianshu.com/p/b4962a2c9584)、[ELF中可以被修改又不影响执行的区域](https://www.jianshu.com/p/9ce47c938706)，于是自己动手写了一个crackme，自我感觉么么哒。 但是不知道在大牛眼中是啥样，于是在群里投放了这个crackme，最终有大牛指出，没乱用，自己按照他的方法看了一下，真是没卵用啊。

源码地址： https://github.com/difcareer/CrackMe apk地址：https://github.com/difcareer/CrackMe/raw/master/app/src/main/misc/crackme.apk

先说一下我的加固思路：

1. 第一层加固： 基本上和前面两个链接中的一样：使用encpypt对sub_1527(核心函数)进行加密（字节取反），然后在.init_array中对函数进行内存解密。
2. 第二层加固： 把ELF中可以改动又不影响执行的地方都改了。这个主要是用来吓唬一下小白，稍微懂一点ELF格式的就能修复（不让ida报错）。我的处理方法是把没有映射到段的节和节区头部表直接删除，修改了ELF头部的e_shoff、e_shentsize、e_shnum、e_shstrndx。

看看效果：

在IDA6.6中，直接不能识别ELF文件，只提示了Binary。

在IDA6.8中，仍然能够识别ELF，但是有报错，IDA6.8比6.6确实增强了不少。

![img](https://ask.qcloudimg.com/http-save/yehe-2930595/uewm86zo90.png?imageView2/2/w/1620)

Paste_Image.png

以前看到这样的弹窗直接被吓尿。

再看看核心函数的代码，一团乱：

![img](https://ask.qcloudimg.com/http-save/yehe-2930595/32gzts835p.png?imageView2/2/w/1620)

Paste_Image.png

因为是我写的代码，所以我想象的破解方法是：1. 先修复so，让IDA不报错，2. 找到解密函数，确认被加密函数的地址空间，dump内存，合并修复。

其实不用，直接使用空白IDA attach 进程，这里需要使用IDA6.8，IDA会根据内存构建视图，此时内存中是已经解密的内容，发现核心函数直接被还原了：

![img](https://ask.qcloudimg.com/http-save/yehe-2930595/qjrdrsidef.png?imageView2/2/w/1620)

Paste_Image.png

稍微调试一下，就能找到flag：

![img](https://ask.qcloudimg.com/http-save/yehe-2930595/x86zt7e4ob.png?imageView2/2/w/1620)

Paste_Image.png

这种静态加密的方式，在动态调试面前，真是没有一点卵用。

那么，牛逼的加固应该是怎么样的呢？

我想到的是：

1. 增加调试检测，然而没有卵用，可以被nop掉。
2. 执行前解密，用完从内存中移除，防止dump，这样也没有卵用，调试断下来，可以慢慢dump。

暂时想不到其他的了，欢迎大家多多发挥探讨。