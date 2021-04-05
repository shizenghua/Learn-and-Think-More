# LLDB image使用

url:https://medium.com/@warmap_/image-604f23175bc2

首先来看下**image**命令，**image**命令是**target modules**命令的简版，**image**命令是专门用来查看modules信息的命令。**modules**可以由很多东西组成，包括main可执行程序、frameworks、插件等。常见的大部分**image**就是动态库，例如UIKit。

**image**命令在查看私有库的类和方法方面非常的方便。

## modules

打开随书带的**Signals**项目，在模拟器上运行。暂停项目并在lldb键入如下命令

```
image list
```

这个命令会打印出所有加载的modules，类似下面的输出

![img](https://miro.medium.com/proxy/1*J2emz3bCSYTFwS9x8GG0xg.jpeg)

第一个module是应用的主二进制文件Signals。第二和第三个是有关动态链接器（dyld）的，这两个modules负责把你的应用和所需动态库加载到内存里。下面还有很多动态库，我们现在只关注感兴趣的几个。在lldb键入下面的命令

```
image list Foundation
```

你会得到类似下面的输出

![img](https://miro.medium.com/proxy/1*SaCSrjqtIQg4gjMK-v0vzw.jpeg)

这是一个对于查看module信息很有用的方式。下面我们来解释下打印的信息

1. 第一个被打印出来的是module的UUID（E5391C7B-0161–33AF-A5A7–1E18DBF9041F）。UUID对查找对应符号文件至关重要，也是这个版本Foundation的唯一的标识。
2. UUID后面的就是加载地址了，它标识了这个module被加载到了Singals程序空间的哪块位置。
3. 最后就是这个module在磁盘上的完整路径了。

下面我们在深入看一下UIKit，在LLDB键入如下命令

```
image dump symtab UIKitCore -s address
```

这会打印出UIKitCore的符号表信息。内容非常多（我这打印了119274项），这条命令的-s参数表明是将私有库UIKitCore的方法按照其内存地址顺序来输出的。

![img](https://miro.medium.com/proxy/1*aiPcjq8PZnq4vpLQOC4YZw.jpeg)

里面信息很多，我们需要一个更高效的方式来找我们感兴趣的代码。**image lookup**正是我们所需要的，在键入下面的命令

```
image lookup -n "-[UIViewController viewDidLoad]"
```

这样会只打印**UIViewController**的**viewDidLoad**方法的相关信息。你会看到如下的输出

![img](https://miro.medium.com/proxy/1*El-S8HR2wlzFqRusodhMXQ.jpeg)

这还不错，但内容非常单一。这时我们可以用正则的方式来打印我们想要的内容。利用下面的命令

```
image lookup -rn "UIViewController"
```

输出如下，明显多了很多，类似UIViewControllerBuiltinTransitionViewAnimator都会被打印，因为它包含了**UIViewController**

![img](https://miro.medium.com/proxy/1*JUuB6rljAdnYtYNpBONgdA.jpeg)

我们可以改进一下，让它只输出**UIViewController**的方法，键入下面命令

```
image lookup -rn '\[UIViewController\ '
```

也可以下面这种方式，用**\s**来标识一个空格，这样也就不用单引号了。但效果是一样的。

```
image lookup -rn \[UIViewController\s
```

看起来上面的命令还OK，但是categories的方法并没有打印。分类应该是**UIViewController(CategoryName)**这样形式，用下面这个命令来查看分类的方法

```
image lookup -rn '\[UIViewController\(\w+\)\ '
```

这有些复杂了，开头的**\**表示我们就是要**[**这个字符，然后是**UIViewController**，在就是字符**(**,然后**\w**表示一个或多个由字母数字或下划线组成的字符串，然后在跟一个**)**，最后在跟一个空格。

这些正则的知识会帮你检索出任何被加载进这个应用的module的方法，不管是私有还是公开。而且上面打印的不止有公有和私有方法，还有UIViewController从父类继承的方法。

## 检索代码/Hunting for code

不管你是要查询共有还是私有方法，有时候搞清楚编译器是如何给一个方法创建方法名也是很有趣的。因为上面你只是单纯的打印出了UIViewController的所有方法，而有的时候知道编译器是怎么生成代码的对你在哪里和怎么打断点都有更好的认知提高。来举一个具体又有意思的case — block的方法签名

那怎么搜索一个block的方法签名最合适呢？鉴于你没有头绪去哪里检索block是怎么命名的，在block里打个断点是个不错的开始。

打开**UnixSignalHandler.m**，找到单例方法，并在如下图的位置打上断点

![img](https://miro.medium.com/proxy/1*WbNVms9kT2ggc6sZSc0F4A.jpeg)

build and run，Xcode会停到断点处，看下调试窗口最上面的调用栈

![img](https://miro.medium.com/proxy/1*AIh05WDpznogjQ18cSY6FA.jpeg)

在调试窗口你能看到你的调用栈，我们可以通过下面命令打印出第0帧

```
frame info
```

你会得到类似下面的输出

![img](https://miro.medium.com/proxy/1*tGrOj3usVw6fPWuHnCl1PQ.jpeg)

正如所见，完整的方法名是 **__34+[UnixSignalHandler sharedHandler]_block_invoke**。其中**_block_invoke**看起来是我们做block方法名匹配的不二选择，键入如下命令

```
image lookup -rn _block_invoke
```

这个正则查找把所有包含**_block_invoke**的方法名都打印出来，这也就是所有加载到程序里的block了，包括来自UIKit，Foundation，iPhoneSimulator SDK等。你应该只搜索Signals module里面的。在键入如下命令

```
image lookup -rn _block_invoke Signals
```

回车后并没有任何输出？打开Xcode右侧的panel，点击**File Inspector**,或者使用快捷键**⌘ + Option + 1**。你会看到**UnixSignalHandler.m**实际被编译到了Commons framework，所以我们要找的block是在Commons module

![img](https://miro.medium.com/proxy/1*Dood5RB_vchKOEKFjO0CxQ.jpeg)

在键入下面的命令

```
image lookup -rn _block_invoke Commons
```

会得到下面的输出

![img](https://miro.medium.com/proxy/1*5QWH5gcCBNotl79LUYb96Q.jpeg)

现在我们有了Commons framework里所有的block名，让我们来打个断点试下。键入如下命令

```
rb appendSignal.*_block_invoke -s Commons
```

会有如下输出

![img](https://miro.medium.com/proxy/1*DMtd1e4hhPbjILBZWI3h2A.jpeg)

这样做会在appendSignal方法里所有的的block打上断点，恢复程序运行。然后在开一个终端输入如下命令

```
pkill -SIGIO Signals
```

这个发给Signals的信号将会被执行，也就是更新tableview显示一条对应的信息。但还没有执行，因为命中了我们刚才打的正则断点。

第一个命中的断点在
`__38-[UnixSignalHandler appendSignal:sig:]_block_invoke`
继续运行项目，你又会命中一个断点
`__38-[UnixSignalHandler appendSignal:sig:]_block_invoke_2`
这个和第一个相比有点意思，多了一个2.原因是编译器是基于 _block_invoke来给函数名为function_name内的block命名的，当block多于1个时，就会被加上后缀.

基于前面章节的知识，**frame variable**命令会打印出对应函数的所有本地变量。下面键入这个命令，看看这个block会有哪些内容

![img](https://miro.medium.com/proxy/1*CT0D09r31Q4sFNJxb_24Uw.jpeg)

会发现有些读取内存失败，这时键入**n**单步执行一下，再执行**frame variable**，这次你会发现如下的输出

![img](https://miro.medium.com/proxy/1*-weuL1ppV-Lbx_nobw1stA.jpeg)

你需要单步执行一句，这样block才能执行一些初始化逻辑，也称为函数序言。函数序言是汇编相关的主题，section 2会讲到。

__block_literal_5 也就是第一个对象正好指向当前执行的block，后面是sig和siginfo也就是被调block所属函数的参数。那这两个参数怎么传入block的呢？当block创建的时候，编译器就是知道block要用这两个参数，然后它创建了一个函数并接受了这两个参数。当block被调用时，这个函数也就被调用了，并把那两个参数传了进去。

在lldb键入如下命令

```
image lookup -t __block_literal_5
```

会得到如下输出

![img](https://miro.medium.com/proxy/1*rpB_RKBnt1eFdbNQcToFPQ.jpeg)

这就是定义这个block的对象！
如你所见，这就像一个头文件告知了你这个block的内存是如何布局的。你可以通过把这块内存转为__block_literal_5类型来打印出block引用的变量
先用如下命令把栈帧的变量都打出来

```
frame variable
```

然后再用如下命令把__block_literal_5对象打印出来

```
po ((__block_literal_5 *)0x00006000005a5680)
```

得到如下输出

![img](https://miro.medium.com/proxy/1*PJL0dGij2ycNlSI2ctDKPg.jpeg)

如果你的打印不是这样，请确保你转为__block_literal_5类型打印的地址，在你每次运行项目是不一样。（这里lldb有个bug）

> Note: Bug alert in lldb-900.0.57 where LLDB will incorrectly dereference the __block_literal_5 pointer when executing the frame variable command. This means that the pointer output of (__block_literal_5 *) will give the class NSMallocBlock instead of the instance of NSMallocBlock. If you are getting the class description instead of an instance description, you can get around this by either referencing the RDI register immediately at the start of the function, or obtain the instance of the **NSMallocBlock** via x/gx ‘$rbp — 32’ if you are further into the function.”

现在你可以查看__block_literal_5结构体的成员了，键入如下命令

```
p/x ((__block_literal_5 *)0x00006000005a5680)->__FuncPtr
```

这会打印出block的函数指针，如下图

![img](https://miro.medium.com/proxy/1*HpTMF60lb3tEZtaTWr844Q.jpeg)

block的这个函数指针，指向block每次被调用时，要执行的函数。也就是现在正在被执行的地址。你可以通过下面的命令（入参为这个地址）来确认就是当前函数的地址。

![img](https://miro.medium.com/proxy/1*XjaUI4uIgMl4m1B9IzZLXQ.jpeg)

image lookup命令的 -a（address）选项是找出地址对应符号的选项。

我们在打印一下这个block结构体中的sig

![img](https://miro.medium.com/proxy/1*uaf1cQHMHJlf1kBq-a2AUA.jpeg)

这会打印出block所在函数传入的sig参数值。
再看一下结构体中还有一个UnixSignalHandler的变量self，这是为什么呢？看下block的源码你会发现

![img](https://miro.medium.com/proxy/1*PBOZKbRpajpdh0efFB3gdQ.jpeg)

block中用了self，所以被block捕捉到了。

顺便说一句，你可以通过如下p命令打印出整个结构体

```
p *(__block_literal_5 *)0x00006000005a5680
```

![img](https://miro.medium.com/proxy/1*VOB_oaifcu-RM7VM3hYGuQ.jpeg)

使用**image dump symfile**传入module，例如`image dump symfile Commons`是研究一个未知模块很好的方式。也是一个理解编译器如何利用源码生成底层code的好办法。
而且还可以检测block是如何引用外部变量的，这对处理block循环引用无疑有很大帮助。

## Snooping around（自由探索下）

你已经具备了静态调试私有类的成员的能力，但刚才block的内存地址我们还是要在动态调试下进一步窥探下。用**po**打印下刚才的地址

![img](https://miro.medium.com/proxy/1*vFrNLnwYH_Hf6mDwFQ8Jnw.jpeg)

LLDB会打印出一个Objective-C类。类名**NSMallocBlock**。现在你已经有能力dump出不管是公共类还是私有类的方法表了，下面我们看下这个**NSMallocBlock**的方法表，lldb键入下面命令

```
image lookup -rn __NSMallocBlock__
```

然而并没有任何输出，🤷‍♂️。这意味着**NSMallocBlock**没有重写父类的方法，自己也没有方法。那我们看下它的父类

![img](https://miro.medium.com/proxy/1*hijQQ_B8Xjrg3Oz91xSHrA.jpeg)

父类名称只是少了后面的下划线，我们再试试这个父类有方法嘛

![img](https://miro.medium.com/proxy/1*hELT7YhrMbz2tBLSDDfuqw.jpeg)

这次dump出得方法，看似都是和内存管理有关的。我们再看它的父类有啥方法

![img](https://miro.medium.com/proxy/1*buaUnbdZnMjR4gMnimux8A.jpeg)

![img](https://miro.medium.com/proxy/1*Y9EaPdRjjswI3PydgqSt5A.jpeg)

这次打印出的方法中有个**invoke**看起来很有意思。我们尝试用这个block实例调用一下**invoke**，因为我们要用这个block做实验，所以不希望这个block被释放掉，简单的办法就是**retain**一下。然后再继续我们的探索

![img](https://miro.medium.com/proxy/1*d0h22R-oIdxOuS5oP5tdzg.jpeg)

上面的打印说明我们的block又被调到了，✌️。能成功调用是因为本身block已经能成功执行了，因为我们现在断点就停在这，也就是说明block的执行条件都是准备好的了。

上面探索公有和私有类以及其实例方法的方式，是我们学习一个程序背后是如何执行的很好的方式。后面你还会用到这些方式进行分析，并在汇编层面看看是如何执行的，从而进一步接近源码的执行逻辑。

## Private debugging mehtods

**image lookup**在检索私有方法时非常好用，就像你查看公有方法一样。但还是有一些隐藏方法，但对我们的调试非常有用。
类如，以**_**开头的方法，也意味着就是私有方法。
下面我们检索一下OC中所有以**_**开头并包含“description”的方法，重新运行我们的项目，并在单例断点停住时，键入以下命令

```
image lookup -rn (?i)\ _\w+description\]
```

先简单解释下正则的**(?i)**这部分，这个说明这个正则是大小写不敏感。如果你还是不太清楚正则那么这个[文档](https://docs.python.org/2/library/re.html)就是你需要的。

回车执行上面命令，仔细查看输出的内容，往往你要的答案就在这一堆枯燥的输出里。你会发现UIKitCore中有一个名为**IvarDescription**的NSObject分类，我们通过下面命令单独查看下这个分类

```
image lookup -rn NSObject\(IvarDescription\)
```

控制台会打印出这个分类的所有方法。

![img](https://miro.medium.com/proxy/1*BXzs2yGZmCIwN9ekksDedw.jpeg)

重点看下这几个
_ivarDescription
_propertyDescription
_methodDescription
_shortMethodDescription

因为这是NSObject的分类，它的所有子类都能用这个方法，那几乎是所有类了。用下面的命令执行一下

```
po [[UIApplication sharedApplication] _ivarDescription]
```

你会得到一大串的输出，因为UIApplication持有了很多实例变量。浏览一遍，你会发现一个对私有类UIStatusBar的引用，我们看下这个类有啥set方法，输入下面的命令

```
image lookup -rn '\[UIStatusBar\ set'
```

回车后输出了所有的set方法，为了也能复写父类方法，我们看一下它的父类。我们试下父类是不是UIView。

![img](https://miro.medium.com/proxy/1*gjeD6H_iqcX4uvQqYvuo8g.jpeg)

或者用superclass也能看到它的父类名称。正如上图所示，它是UIView的子类，我们来换下背景颜色是一下。首先打印出这个对象

![img](https://miro.medium.com/proxy/1*sYzd_p1VkW-OnWAUzeRcSA.jpeg)

然后复制实例的地址，在执行下面的命令

![img](https://miro.medium.com/proxy/1*PiPHo7BaKzfZf8uJt5G9kQ.jpeg)

最后删掉所有的断点

![img](https://miro.medium.com/proxy/1*m2W9XdFPa6RmlWkAn3N0-Q.jpeg)

现在恢复应用的运行你会发现状态栏变紫了

![img](https://miro.medium.com/proxy/1*w5SfMApeWMuNQB1WWN9N0Q.jpeg)