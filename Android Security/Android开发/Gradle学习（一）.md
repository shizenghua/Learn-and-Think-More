## Groovy介绍

- Groovy中支持动态类型，即定义变量的时候可以不指定其类型。(Groovy中，变量定义可以使用关键字def。def不是必须的，但是为了代码清晰，建议还是使用def关键字)

  ```
  def a = 5;
  def b = "groovy"
  ```

- 函数的定义，我们也无需进行参数类型的声明.

  ```
  String testFunction(arg1,arg2){//无需指定参数类型
    ...
  }
  ```

- 除了变量定义可以不指定类型外，Groovy中函数的返回值也可以是无类型的。比如：

  ```
  //无类型的函数定义，必须使用def关键字
  def  nonReturnTypeFunc(){
       last_line   //最后一行代码的执行结果就是本函数的返回值
  }
  
  //如果指定了函数返回类型，则可不必加def关键字来定义函数
  String  getString(){
     return "I am a string"
  }
  ```

- 函数返回值：Groovy的函数里，可以不使用return xxx来设置xxx为函数返回值。如果不使用return语句的话，则函数里最后一句代码的执行结果被设置成返回值。比如

  ```
  def getSomething(){
  
        "getSomething return value" //如果这是最后一行代码，则返回类型为String
  
        1000 //如果这是最后一行代码，则返回类型为Integer
  }
  ```

  注意，如果函数定义时候指明了返回值类型的话，函数中则必须返回正确的数据类型，否则运行时报错。如果使用了动态类型的话，你就可以返回任何类型了。

- 除了每行代码不用加分号外，Groovy中函数调用的时候还可以不加括号。比如：
  `println("test") 等价于 println "test"`

- 函数调用支持 参数名：参数值方式调用
  `apply plugin: 'com.android.library'`
  plugin:参数名，’com.android.library’：参数值

- 强大字符串支持功能

  ```
  //单引号''中的内容严格对应Java中的String，不对$符号进行转义
  def singleQuote='I am $ dolloar'  //输出就是I am $ dolloar
  
  //双引号,则它会$表达式先求值。
  def doubleQuoteWithoutDollar = "I am one dollar" //输出 I am one dollar
  def x = 1
  def doubleQuoteWithDollar = "I am $x dolloar" //输出I am 1 dolloar
  
  //通过换行实现每一行的间距
  str3 = '''begin
      line1
      line2
  end'''
  ```

- 闭包

  （英语：Closure），又称词法闭包（Lexical Closure）或函数闭包（function closures），是引用了自由变量的函数。这个被引用的自由变量将和这个函数一同存在，即使已经离开了创造它的环境也不例外。所以，有另一种说法认为闭包是由函数和与其相关的引用环境组合而成的实体。对于闭包的实现，从函数式编程的角度来看就为了解决一个输入对应一个输出的问题。例如当我们想实现一个加法，我们必须通过传递两个参数来实现，但是借助于函数式编程，我们可以做到只传递一个参数。

  ```
  function plusAny(first) {
     return function(second) {
          return first + second;
     }
  }
  
  var longLiveSeniorFunc = plusAny(1);
  
  longLiveSeniorFunc(1);
  ```

**闭包，是一种数据类型，它代表了一段可执行的代码**。其外形如下：

```
def aClosure = {//闭包是一段代码，所以需要用花括号括起来..  
    String param1, int param2 ->  //这个箭头很关键。箭头前面是参数定义，箭头后面是代码  
    println "this is code" //这是代码，最后一句是返回值，  
   //也可以使用return，和Groovy中普通函数一样  
}
```

简而言之，Closure的定义格式是：

```
def xxx = {paramters -> code}  //或者  
def xxx = {无参数，纯code}  这种case不需要->符号
```

说实话，从C/C++语言的角度看，闭包和函数指针很像。闭包定义好后，要调用它的方法就是：
闭包对象.call(参数) 或者更像函数指针调用的方法：
闭包对象(参数)
比如：

```
aClosure.call("this is string",100)  或者  
aClosure("this is string", 100)
```

如果闭包没定义参数的话，则隐含有一个参数，这个参数名字叫it，和this的作用类似。it代表闭包的参数。
比如

```
def greeting = { "Hello, $it!" }
assert greeting('Patrick') == 'Hello, Patrick!'
```

等同于：

```
def greeting = { it -> "Hello, $it!" }
assert greeting('Patrick') == 'Hello, Patrick!'
```

但是，如果在闭包定义时，采用下面这种写法，则表示闭包没有参数.
`def noParamClosure = { -> true }`
这个时候，我们就不能给noParamClosure传参数了。

- Groovy中，当函数的最后一个参数是闭包的话，可以省略圆括号。比如

  ```
  def  testClosure(int a1,String b1, Closure closure){
        //do something
        closure() //调用闭包
  }
  那么调用的时候，就可以免括号！
  testClosure (4, "test", {
     println "i am in closure"
  } )  //外面的括号可以不写..
  ```

  注意，这个特点非常关键，因为以后在Gradle中经常会出现这样的代码。

  ![img](https://ws2.sinaimg.cn/large/006tNc79ly1foamyow3zkj309k03aglh.jpg)

  省略圆括号虽然使得代码简洁，看起来更像脚本语言，但是它这经常会让我confuse，以doLast为例，完整的代码应该按下面这种写法：

  ```
  doLast({
     println 'Hello world!'
  })
  ```

  有了圆括号，你会知道 doLast只是把一个Closure对象传了进去。很明显，它不代表这段脚本解析到doLast的时候就会调用println ‘Hello world!’ 。

  但是把圆括号去掉后，就感觉好像println ‘Hello world!’立即就会被调用一样.

## Gradle工作流程

[![img](https://ws2.sinaimg.cn/large/006tNc79ly1foalfwtituj30800ay0t4.jpg)](https://ws2.sinaimg.cn/large/006tNc79ly1foalfwtituj30800ay0t4.jpg)
如上图所示，在一个Project中，除了我们项目自身的代码和资源之外，会有多个与项目构建相关的.gradle文件。
Gradle中，每一个待编译的工程都叫一个Project。每一个Project在构建的时候都包含一系列的Task。比如一个Android APK的编译可能包含：Java源码编译Task、资源编译Task、JNI编译Task、lint检查Task、打包生成APK的Task、签名Task等。

Gradle的工作流程如下图所示，在每一个工作流程的前后，我们都可以进行一些hook操作，来满足自己的需求。
[![img](https://ws3.sinaimg.cn/large/006tNc79ly1foalgl23j6j30q7065js9.jpg)](https://ws3.sinaimg.cn/large/006tNc79ly1foalgl23j6j30q7065js9.jpg)
Gradle工作包含三个阶段：

- 首先是初始化阶段。对我们前面的multi-project build而言，就是执行settings.gradle
- Initiliazation phase的下一个阶段是Configration阶段。
- Configration阶段的目标是解析每个project中的build.gradle。比如multi-project build例子中，解析每个子目录中的build.gradle。在这两个阶段之间，我们可以加一些定制化的Hook。这当然是通过API来添加的。
- Configuration阶段完了后，整个build的project以及内部的Task关系就确定了。一个Project包含很多Task，每个Task之间有依赖关系。Configuration会建立一个有向图来描述Task之间的依赖关系。所以，我们可以添加一个HOOK，即当Task关系图建立好后，执行一些操作。
- 最后一个阶段就是执行任务了。当然，任务执行完后，我们还可以加Hook。

简言之，Gradle有一个初始化流程，这个时候settings.gradle会执行。
在配置阶段，每个Project都会被解析，其内部的任务也会被添加到一个有向图里，用于解决执行过程中的依赖关系。然后才是执行阶段。你在gradle xxx中指定什么任务，gradle就会将这个xxx任务链上的所有任务全部按依赖顺序执行一遍。

## Gradle Wrapper

Gradle Wrapper ，意为 Gradle 的包装，什么意思呢？
假设我们本地有多个项目，一个是比较老的项目，还用着 Gradle 1.0 的版本，一个是比较新的项目用了 Gradle 2.0 的版本，但是你两个项目肯定都想要同时运行的，如果你只装了 Gradle 1.0 的话那肯定不行，所以为了解决这个问题，Google 推出了 Gradle Wrapper 的概念。
就是他在你每个项目都配置了一个指定版本的 Gradle ，你可以理解为每个 Android 项目本地都有一个小型的 Gradle ，通过这个每个项目你可以支持用不同的 Gradle 版本来构建项目。
[![img](https://ws1.sinaimg.cn/large/006tNc79ly1foanvlylrhj312m0fk415.jpg)](https://ws1.sinaimg.cn/large/006tNc79ly1foanvlylrhj312m0fk415.jpg)

## Android 项目包含的 Gradle 配置文件

[![img](https://ws1.sinaimg.cn/large/006tKfTcly1foanyhm51xj308z0ieq44.jpg)](https://ws1.sinaimg.cn/large/006tKfTcly1foanyhm51xj308z0ieq44.jpg)

- 9GAG/app/build.gradle
  这个文件是 app 文件夹下这个 Module 的 gradle 配置文件，也可以算是整个项目最主要的 gradle 配置文件。
- 9GAG/extras/ShimmerAndroid/build.gradle
  每一个 Module 都需要有一个 gradle 配置文件，语法都是一样，唯一不同的是开头声明的是`apply plugin: 'com.android.library'`
- 9GAG/gradle
  这个目录下有个 wrapper 文件夹，里面可以看到有两个文件，我们主要看下 gradle-wrapper.properties 这个文件的内容：
  [![img](https://ws1.sinaimg.cn/large/006tKfTcly1foaouah7rdj311q09cgnu.jpg)](https://ws1.sinaimg.cn/large/006tKfTcly1foaouah7rdj311q09cgnu.jpg)
  可以看到里面声明了 gradle 的目录与下载路径以及当前项目使用的 gradle 版本，这些默认的路径我们一般不会更改的，这个文件里指明的 gradle 版本不对也是很多导包不成功的原因之一。
- 9GAG/build.gradle
  这个文件是整个项目的 gradle 基础配置文件，默认的内容就是声明了 android gradle plugin 的版本。
  [![img](https://ws2.sinaimg.cn/large/006tKfTcly1foaow75wlsj30tw0gi40d.jpg)](https://ws2.sinaimg.cn/large/006tKfTcly1foaow75wlsj30tw0gi40d.jpg)
- 9GAG/settings.gradle
  这个文件是全局的项目配置文件，里面主要声明一些需要加入 gradle 的 module，我们来看看 9GAG 该文件的内容：
  [![img](https://ws1.sinaimg.cn/large/006tKfTcly1foaoxckd1bj31800piq6r.jpg)](https://ws1.sinaimg.cn/large/006tKfTcly1foaoxckd1bj31800piq6r.jpg)

## 参考链接

http://www.infoq.com/cn/articles/android-in-depth-gradle
http://android.walfud.com/android-gradle-看这一篇就够了/