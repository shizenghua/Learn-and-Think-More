# ELF中可以被修改又不影响执行的区域

url：https://www.jianshu.com/p/9ce47c938706

看雪上[这篇文章](http://bbs.pediy.com/showthread.php?t=191649)讲述了两种对so进行加固的方法：1. 分离section，对整个section进行加密。2.在.text section直接寻找目标函数并进行加密，两种方式的实践代码见文末。
 这里讲一些我在学习过程中的一些额外发现，如有理解不对的地方，欢迎斧正。
 一. 关于ELF的链接视图和装载视图（执行视图）。在所有介绍ELF文件格式的文档中，都会出现这样一张图：

![img](https:////upload-images.jianshu.io/upload_images/1784193-fcbd44c59c780cac.png!small?imageMogr2/auto-orient/strip|imageView2/2/w/362/format/webp)

Clipboard Image.png


 当初看这张图并没有很深的认识。现在回过头看看，发现有了一些新的认识。
 在[C代码 从源代码到可执行文件——编译全过程解析](https://www.jianshu.com/p/8dc5b0689b53)中，我们看到C程序经过 预处理、编译、汇编、链接 之后，形成可执行文件或者共享目标文件。其中的链接环节，就会需要节区头部表。当然这里说的是静态链接。静态链接完成后，节区头部表在后续的加载执行中已经不会再用到了。
 我们再来看看动态链接：动态链接的典型场景就是一个可执行文件加载了一个so文件，并执行了so的函数。详细解释如下(摘自[ELF文件格式](http://blog.chinaunix.net/attachment/attach/26/40/46/9726404697228d82cda2af11366fa7722d3a4f1a58.pdf))：





```css
3.8.3 动态链接

3.8.3.1 程序解释器 
    可执行文件可以包含 PT_INTERP 程序头部元素。在 exec() 期间，系统从
PT_INTERP 段中检索路径名，并从解释器文件的段创建初始的进程映像。也就是说，
系统并不使用原来可执行文件的段映像，而是为解释器构造一个内存映像。接下来是解
释器从系统接收控制，为应用程序提供执行环境。 
    解释器可以有两种方式接受控制。� 
      接受一个文件描述符，读取可执行文件并将其映射到内存中� 
      根据可执行文件的格式，系统可能把可执行文件加载到内存中，而不是为解释器提
供一个已经打开的文件描述符。 
    解释器可以是一个可执行文件，也可以是一个共享目标文件。 
    共享目标文件被加载到内存中时，其地址可能在各个进程中呈现不同的取值。系统
在 mmap 以及相关服务所使用的动态段区域创建共享目标文件的段。因此，共享目标
解释器通常不会与原来的可执行文件的原始段地址发生冲突。 
    可执行文件被加载到内存中固定地址，系统使用来自其程序头部表的虚拟地址创建
各个段。因此，可执行文件解释器的虚拟地址可能会与原来的可执行文件的虚拟地址发
生冲突。解释器要负责解决这种冲突。

3.8.3.2 动态加载程序 

    在构造使用动态链接技术的可执行文件时，连接编辑器向可执行文件中添加一个类
型为 PT_INTERP 的程序头部元素，告诉系统要把动态链接器激活，作为程序解释器。系
统所提供的动态链接器的位置是和处理器相关的。 
    Exec() 和动态链接器合作，为程序创建进程映像，其中包括以下动作：
     (1). 将可执行文件的内存段添加到进程映像中；
     (2). 把共享目标内存段添加到进程映像中； 
     (3). 为可执行文件和它的共享目标执行重定位操作； 
     (4). 关闭用来读入可执行文件的文件描述符，如果动态链接程序收到过这样的
文件描述符的话；
     (5). 将控制转交给程序，使得程序好像从 exec 直接得到控制。 
    链接编辑器也会构造很多数据来协助动态链接器处理可执行文件和共享目标文件。
这些数据包含在可加载段中，在执行过程中可用。如：�
     类型为 SHT_DYNAMIC 的 .dynamic 节区包含很多数据。位于节区头部的结构保存
了其他动态链接信息的地址。� 
     类型为 SHT_HASH 的 .hash 节区包含符号哈希表。� 
     类型为 SHT_PROGBITS 的 .got 和 .plt 节区包含两个不同的表：全局偏移表和过
程链接表。 
    因为任何符合 ABI 规范的程序都要从共享目标库中导入基本的系统服务，动态链
接器会参与每个符合 ABI 规范的程序的执行。...
```

也就是说，动态链接时通过段去解析出对应的section的内容。即：动态链接时也没有使用到节区头部表。
 因此，我们可以猜测，节区头部表在程序执行过程中并不会被用到，既然不会被用到，ELF头部中的e_shoff、e_shentsize、e_shnum和e_shstrndx可以随意修改而不会影响到程序执行。

二. 关于节和段。在链接视图中，数据被划分为节：



```undefined
  节区满足以下条件：
   (1). 目标文件中的每个节区都有对应的节区头部描述它，反过来，有节区头部不意
味着有节区。
   (2). 每个节区占用文件中一个连续字节区域（这个区域可能长度为 0）。 
   (3). 文件中的节区不能重叠，不允许一个字节存在于两个节区中的情况发生。 
   (4). 目标文件中可能包含非活动空间（INACTIVE SPACE）。这些区域不属于任何
头部和节区，其内容未指定。
```

在加载视图中，数据被划分为段：



```undefined
    可执行文件或者共享目标文件的程序头部是一个结构数组，每个结构描述了一个段
或者系统准备程序执行所必需的其它信息。目标文件的“段”包含一个或者多个“节区”，
也就是“段内容（Segment Contents）”。程序头部仅对于可执行文件和共享目标文件
有意义。
```

看一个实际的例子：
 readelf的输出



![img](https:////upload-images.jianshu.io/upload_images/1784193-f7b0028c366d85c3.png!small?imageMogr2/auto-orient/strip|imageView2/2/w/690/format/webp)

Clipboard Image.png

转为直观的表格



![img](https:////upload-images.jianshu.io/upload_images/1784193-81e6460b5f059ac4.png!small?imageMogr2/auto-orient/strip|imageView2/2/w/626/format/webp)

Clipboard Image.png

从图中可以看出：a. 一个节区可以同时属于多个段区b. 一个段区可以包含多个节区c. 段区由多个节区组成时，这些节区是连续的d. 我们看到.comments、.note.gnu.gold-ve、.ARM.attributes、.shstrtab 并没有被映射到段，因此可以猜测这些区间的内容修改不会影响程序的执行。

三. 总结ELF中可以被修改又不影响执行的区域
 a. ELF头部中的：e_shoff、e_shentsize、e_shnum、e_shstrndx
 b. 整个section头部表
 c. 没有被映射到段的节（具体会有不同），比如上面的.comments、.note.gnu.gold-ve、.ARM.attributes、.shstrtab。

四. 测试结果
 清零ELF头部的e_shoff、e_shentsize、e_shnum、e_shstrndx



![img](https:////upload-images.jianshu.io/upload_images/1784193-c2eed38efe95b65c.png!small?imageMogr2/auto-orient/strip|imageView2/2/w/618/format/webp)

Clipboard Image.png



清零节区头部表（010Editor中已经解析不出节区表了），清空没有被映射到段的节区



![img](https:////upload-images.jianshu.io/upload_images/1784193-97efe1ea6afaebec.png!small?imageMogr2/auto-orient/strip|imageView2/2/w/621/format/webp)

Clipboard Image.png

替换so，打包签名后，正常运行：



![img](https:////upload-images.jianshu.io/upload_images/1784193-360e84d4fc4fc571.png!small?imageMogr2/auto-orient/strip|imageView2/2/w/634/format/webp)

Clipboard Image.png

测试的apk见：
 https://github.com/difcareer/SoEncrypt2/raw/master/app/src/main/misc/trip.apk

五. 作用

1. 这些区域可以被精心构造，影响到ELF解析工具，比如readelf,IDA等。比如前面看雪那篇文章中提到的分离section方式加固中，因为修改了ELF头部中的section相关的字段，就会导致IDA打开报错。
2. 这些区域可以用来存储自定义的内容，甚至是被移除来压缩ELF。

本文代码参见：
 通过分离section，加密section：[https://github.com/difcareer/SoEncrypt](https://github.com/difcareer/SoEncrypt2)
 通过查找函数并加密函数：https://github.com/difcareer/SoEncrypt2

参考链接：
 http://bbs.pediy.com/showthread.php?t=191649
 [http://www.wjdiankong.cn/android%e9%80%86%e5%90%91%e4%b9%8b%e6%97%85-%e5%9f%ba%e4%ba%8e%e5%af%b9so%e4%b8%ad%e7%9a%84%e5%87%bd%e6%95%b0%e5%8a%a0%e5%af%86%e6%8a%80%e6%9c%af%e5%ae%9e%e7%8e%b0so%e5%8a%a0%e5%9b%ba/](http://www.wjdiankong.cn/android逆向之旅-基于对so中的函数加密技术实现so加固/)
 [http://zke1ev3n.me/2015/12/27/Android-So%E7%AE%80%E5%8D%95%E5%8A%A0%E5%9B%BA/](http://zke1ev3n.me/2015/12/27/Android-So简单加固/)
 http://blog.chinaunix.net/attachment/attach/26/40/46/9726404697228d82cda2af11366fa7722d3a4f1a58.pdf



作者：difcareer
链接：https://www.jianshu.com/p/9ce47c938706
来源：简书
著作权归作者所有。商业转载请联系作者获得授权，非商业转载请注明出处。