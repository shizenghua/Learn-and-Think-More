# Android so库加固加壳方案

url：http://szuwest.github.io/android-soku-jia-gu-jia-ke-fang-an.html



Android应用主要包含资源文件和代码，而代码一般包括Java代码和C/C++代码。Java代码编译后会生成dex文件，而C/C++编译后会生成so文件。Android应用的保护，主要对编译后的dex文件和so文件来保护，防止被别人反编译查看到里面的核心代码和逻辑。

对于Java代码，一般在编译时做代码混淆，编译后的文件名，函数名和变量名会变成一些无意义的名字，这个即使代码被反编译出来，也很难读懂。不过由于调用关系和逻辑都还在，花些时间还是有可能读懂。所以就有了一些防止反编译的方法出现。这就包括了dex加壳或者加密方法。对dex文件保护的方法现在有不少成熟的方案，有不少第三方公司免费提供加固方案。

对于C/C++代码的保护方案，会比Java代码的更麻烦一下。C/C++经过编译后生成so文件，这个so文件同样会能被反编译。由于我们的主要算法都采用C/C++来实现，并生成so文件提供给合作方使用，我们重点说的是C/C++代码和so文件的保护。

对于so文件的保护，可分为有so源代码和无so源码的情况。

## 有源码保护

针对有源代码的情况，可以大致分为代码混淆，Section或者函数加密。

### 代码混淆

代码混淆最简单的方法就是利用宏定义混淆函数名。例如通过一个宏定义把一个有意义的函数名变为一个无意义的字母组合，在编译后有意义的函数名就被替换为无意义的字母组合，增加了被反编译后阅读理解难度。当然这种方法效率太低，最好当然是编译器来做。NDK编译工具并没有提供这种混淆的功能，但是可以利用LLVM-Obfuscator功能来混淆代码。LLVM-Obfuscator是一个开源的专门用于代码混淆的工具。在Android的NDK编译工具中可以集成LLVM-Obfuscator，需要修改交叉工具链的代码和一些配置参数，可以编译出混淆代码。现在网上也有一个专做这种混淆的方案商 叫Safengine。

### Section或者函数加密

so库是一个ELF文件，它是有一定的格式的，包含了ELF header，若干Section header，若干section等。我们可以在代码中将核心函数定义在自定义的一个section中 （通过 __attribute__ ((section (".mytext")))） 。然后编译出来的so文件中就能找到这个自定义的section。我们可以对这个section进行加密。当程序load这个so库的时候，我们需要确保在main函数之前把section部分解密了。解密函数代码是写在so的源代码里，为了确保解密函数优先于main函数，需对加密函数加了一个 __attribute__((constructor))特性声明，会先于main前执行。解密函数需要先找到so的起始地址，获取到section的偏移值和size，然后修改内存操作权限和解密。

对于特定的函数加密，原理跟对自定义section加密的原理是一样，只不过是查找不再是section，而是特定函数名。这里不再展开。

## 无源码保护

对于只有so库，没有其源码的情况下，就无法进行代码混淆。一种简单的方法是破坏ELF header或者删除Section header。因为在动态库的链接过程中，so文件ELF header某些字段是无用的，这些字段可以随意修改。修改了这些字段会导致反编译软件ida打不开这个so文件。同样Section header也是在链接过程中是没有用到的，可以随意删除，也会导致ida打不开这个so文件。不过这种方式容易被修正和破解掉。

还有另外一种方式需要另外一个解密so库来实现。首先对源码so文件特定函数或者section进行加密，然后把解密函数放到另外一个so中。然后程序中需先加载被加密的so文件，然后加载解密so文件，解密so文件加载过程中就执行解密函数，这样确保加密的so中特定函数或者section解密了。

## 第三方解决方案

对于so文件或者apk文件加固加壳保护方法，是比较专业的领域，现在市场上也有一些专门的团队在做。不过主要提供的方案都是针对apk来加固加壳。对于dex文件的加固加壳方案比较成熟，不少第三方都是免费提供。但是对于so文件的加固加壳，都是属于高级功能，需要付费使用。

对于apk（主要是Java代码）加固加密提供第三方服务主要有：

- 腾讯云应用乐固
- 阿里聚安全
- 360加固保
- 爱加密

要做so文件的加固加壳，乐固暂时没有对外提供，阿里聚安全需要先付费。爱加密也提供了so文件的加固加壳，还提供专门的SDK加密方案，不过需要先找他们的人对接洽谈。

总的来说，要做好so文件的加固加壳保护并不是件容易的事，需要对so文件格式，so加载和链接过程有足够多的了解。另外加固和加壳后也不是绝对安全的，还是有可能被破解。不过加固和加壳对于核心算法还是需要的，加大被破解的难度和成本。有需要的话也可以采用第三方的服务。

### 参考资料

[Android LLVM-Obfuscator C/C++ 混淆编译的深入研究](http://blog.csdn.net/wangbaochu/article/details/45370543)

[SO(ELF)文件格式详解](http://blog.csdn.net/jiangwei0910410003/article/details/49336613)

[基于对so中的section加密技术实现so加固](http://blog.csdn.net/jiangwei0910410003/article/details/49962173)

[基于对so中的函数加密技术实现so加固](http://blog.csdn.net/jiangwei0910410003/article/details/49966719)

[简单粗暴的so加解密实现](https://bbs.pediy.com/thread-191649.htm)

[无源码加解密实现 && NDK Native Hook](https://bbs.pediy.com/thread-192047.htm)

[另一种无源码的so加壳实现](https://bbs.pediy.com/thread-222760.htm)

[Android SO 加壳(加密)与脱壳思路](http://blog.csdn.net/jltxgcy/article/details/52205210)

[android-加固方案对比](https://www.niwoxuexi.com/blog/android/article/233.html)