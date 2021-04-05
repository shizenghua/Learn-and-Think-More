# ELF文件格式修复

url：https://www.jianshu.com/p/27bed97a3896

在[IDA动态调试-没啥卵用的静态加固](https://www.jianshu.com/p/d77323dea174)中，我构造了一个畸形的ELF文件，虽然能够糊弄一下IDA的静态分析，但是动态分析无效。

新的疑问随之而来：如前面所述，我在构造畸形ELF文件的时候，直接把section头部表给删除了，还修改了ELF头部和section相关的字段，为啥不会影响执行呢？这个问题在[Android Linker学习笔记[转\]](https://www.jianshu.com/p/c648f1aacfc2)中有答案：即在动态链接时，使用了动态链接表来索引原先需要使用section头部表获取的内容，比如符号表。

问题又随之而来，既然没有了section头部表，那我如何能够定位到.init_array节区的位置呢（这个对于逆向分析很重要）？又该如何定位到符号表呢？

这两个问题在Android Linker的那篇文章中也有答案，原来，原先需要通过section头部表来获取的节区，现在可以通过DYNAMIC段获取，DYNAMIC段中只包含了.dynamic节区，其实也就是通过.dynamic节区来获取。

先看看.dynamic节区的定义：



```cpp
typedef struct {
        Elf32_Sword d_tag;
        union {
                Elf32_Word      d_val;
                Elf32_Addr      d_ptr;
                Elf32_Off       d_off;
        } d_un;
} Elf32_Dyn;

typedef struct {
        Elf64_Xword d_tag;
        union {
                Elf64_Xword     d_val;
                Elf64_Addr      d_ptr;
        } d_un;
} Elf64_Dyn;
```

说白了.dynamic节区是一个Elf32_Dyn数组

Elf32_Dyn中的d_tag非常重要，它标示了各种数据类型，其后的d_un也是根据d_tag进行解释的。比如我们想要知道的.init_array就有对应的DT_INIT_ARRAY，具体的d_tag及解释参见[Dynamic Section](https://docs.oracle.com/cd/E23824_01/html/819-0690/chapter6-42444.html#scrolltoc)。

我们想要获取的.init_array，符号表，字符串表等，都可以通过Elf64_Dyn中的对应项目获取到，这里我就不展开说了，可以自行学习一下。

当然，我也提供一个小工具供大家解析畸形的ELF。

到目前为止，我发现对文件格式解析最强大的，当属010Editor（假如有比这个好的，请告知我一下，非常感谢）。网上可以下载到解析ELF文件的Template，但这个Template对于畸形ELF不做深入解析，为此，我花了1天时间学习了010Editor的语法，发现它的模板真的是非常强大。最终也完成了对ELF Template的增强。

直接看看效果：

![img](https:////upload-images.jianshu.io/upload_images/1784193-c11c5e1ed92db94d.png?imageMogr2/auto-orient/strip|imageView2/2/w/778/format/webp)

新增对动态链接表的解析

![img](https:////upload-images.jianshu.io/upload_images/1784193-93bb230a03f7e70a.png?imageMogr2/auto-orient/strip|imageView2/2/w/778/format/webp)

新增对动态字符串表的解析

![img](https:////upload-images.jianshu.io/upload_images/1784193-ae5d65b33c471c88.png?imageMogr2/auto-orient/strip|imageView2/2/w/799/format/webp)

新增对.initArray .finiArray 符号表的解析

同时还增加了对多个Loadable段的着色。

模板地址：
 https://github.com/difcareer/010templates/blob/master/ELFTemplate.new.bt

我并没有把所有的d_tag都解析完，看过动态链接过程就可以仿照着解析出来，欢迎有兴趣的同学进行完善并提交，也欢迎指出其中的bug。

写到这里，才发现标题写的是ELF文件的修复，我也不是真的要去修复被删除的节区头部表，我觉得要修复这个应该是有难度的，主要是信息会有缺失。 但既然我们能有别的途径来索引到我们想要访问的位置，也可以变相说成是对ELF的修复吧。



