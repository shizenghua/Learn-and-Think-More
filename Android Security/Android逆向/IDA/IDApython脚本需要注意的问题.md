IDApython官方最全面技术文档：https://www.hex-rays.com/products/ida/support/idapython_docs/

IDApython进行二进制代码审计：https://www.anquanke.com/post/id/197784

IDA7.5的IDApython库函数idc不支持之前python2.7的IDApython脚本idc函数。关于IDA7.2和IDA7.5的IDApython脚本API变化见官网IDApython——API变化

IDAPython 由三个分离的模块组成,他们分别是 idc,idautils 和 idaapi。
三个模块的作用分别是：
idc(注意大小写,不是 IDA 中的 IDC)是一个封装了 IDA 的 IDC 的兼容性模块,idautils 是 IDA 的高级实用功能模块,idaapi 允许了我们访问更加底层的数据。


在IDA中诸如此类红色地址显示的代码，是没有被标记为函数的代码或数据。


在IDA中诸如此类黄色地址显示的内容，isUnknown()函数返回False，isCode(）函数返回True，表明这是代码，但是可以根据idc.FindUnexplored(start,SEARCH_DOWN)函数定位到。这实质应该是未被识别的内容。
————————————————
版权声明：本文为CSDN博主「摔不死的笨鸟」的原创文章，遵循CC 4.0 BY-SA版权协议，转载请附上原文出处链接及本声明。
原文链接：https://blog.csdn.net/qq_43312649/article/details/105860716