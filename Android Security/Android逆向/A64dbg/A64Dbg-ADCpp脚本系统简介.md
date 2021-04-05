## A64Dbg-ADCpp脚本系统简介

![图片](images/640)

IDA有IDC这样的类C脚本系统，Frida嫁接了一层JavaScript脚本系统。虽然用起来还行，但始终不能让人满意。因为与Native打交道，就应该像C/C++那样直白，毕竟操作void *才是Native的精华。受限于C/C++是编译型的静态语言，想要实现像Python/JavaScript那样的便捷性着实不易。

LLDB的表达式倒是可以使用C/C++语句，但是高度依赖类型系统，否则难以写出有效的C/C++表达式。即便如此，还是太弱了，不能写出复杂的完整C/C++函数。

但是，这一切即将成为过去时，我们将在A64Dbg v1.6专业版中引入UnicornVM的时候同时引入ADCpp这个C/C++脚本系统，它是把C/C++定义为了编译型动态语言，使用解释执行的方式运行代码。C/C++最终编译为机器相关的arch.adc字节码，由UnicornVM直接加载执行。

不需要手动指定头文件、不需要手动指定链接库文件、不需要手动加载动态库，你只需要code，剩下的就是交给ADCpp了。

你还可以在A64Dbg端定义数据接收的Python3函数，然后在C/C++端发送给它，类似于Frida JavaScript与Python的交互。



```
// ADCpp snippe
send2ad("Hello, ADCpp~"); // log to a64dbg
send2py("your_pyfn", "Hello, ADCpp~"); // send string to your_pyfn
send2py("your_pyfn", ptr, size); // send buffer to your_pyfn
```

```
# Python3 snippe
def your_pyfn(data):
    print(data) # data is sended by send2ad
```

