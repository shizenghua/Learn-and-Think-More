# Android JNI入门

# JNI

## JNI引入

- JNI : Java本地接口,Java Native Interface, 它是一个协议, 该协议用来沟通Java代码和外部的本地C/C++代码, 通过该协议 Java代码可以调用外部的本地代码, 外部的C/C++ 代码可以调用Java代码;

- C和Java的侧重 :
  C语言 : C语言中最重要的是函数 function;
  Java语言 : Java中最重要的是 JVM, class类, 以及class中的方法;

- C与Java如何交流 :
  JNI规范 : C语言与Java语言交流需要一个适配器, 中间件, 即 JNI, JNI提供了一种规范;
  C语言中调用Java方法 : 可以让我们在C代码中找到Java代码class中的方法, 并且调用该方法;
  Java语言中调用C语言方法 : 同时也可以在Java代码中, 将一个C语言的方法映射到Java的某个方法上;
  JNI桥梁作用 : JNI提供了一个桥梁, 打通了C语言和Java语言之间的障碍;

- JNI中的一些概念 :

  native : Java语言中修饰本地方法的修饰符, 被该修饰符修饰的方法没有方法体;

  Native方法 : 在Java语言中被native关键字修饰的方法是Native方法;

  JNI层 : Java声明Native方法的部分;

  JNI函数 : JNIEnv提供的函数, 这些函数在jni.h中进行定义;

  JNI方法 : Native方法对应的JNI层实现的 C/C++方法, 即在jni目录中实现的那些C语言代码;

  ## JNI在Android中作用

  JNI可以调用本地代码库(即C/C++代码), 并通过 Dalvik虚拟机 与应用层 和 应用框架层进行交互, Android中JNI代码主要位于应用层 和 应用框架层;

  应用层 : 该层是由JNI开发, 主要使用标准JNI编程模型;

  应用框架层 : 使用的是Android中自定义的一套JNI编程模型, 该自定义的JNI编程模型弥补了标准JNI编程模型的不足;

  JNI是连接框架层 (Framework - C/C++) 和应用框架层(Application Framework - Java)的纽带;

  Android中JNI源码位置 : 在应用框架层中, 主要的JNI代码位于 framework/base目录下, 这些模块被编译成共享库之后放在 /system/lib 目录下;

  NDK与JNI区别 :

- NDK: NDK是Google开发的一套开发和编译工具集, 主要用于Android的JNI开发;

- JNI : JNI是一套编程接口, 用来实现Java代码与本地的C/C++代码进行交互;

  ## JNI编程步骤:

1. 声明native方法 : 在Java代码中声明 native method()方法;
2. 实现JNI的C/C++方法 : 在JNI层实现Java中声明的native方法, 这里使用javah工具生成带方法签名的头文件, 该JNI层的C/C++代码将被编译成动态库;
3. 加载动态库 : 在Java代码中的静态代码块中加载JNI编译后的动态共享库;

# Android中的应用程序框架

## 正常情况下的Android框架

最顶层是Android的应用程序代码, 上层的应用层 和 应用框架层 主要是Java代码, 中间有一层的Framework框架层代码是 C/C++代码, 通过Framework进行系统调用, 调用底层的库 和linux 内核;
[![img](https://ws2.sinaimg.cn/large/006tKfTcly1fo82wjl40oj30fd08xmxm.jpg)](https://ws2.sinaimg.cn/large/006tKfTcly1fo82wjl40oj30fd08xmxm.jpg)

## 使用JNI时的Android框架

绕过Framework提供的调用底层的代码, 直接调用自己写的C代码, 该代码最终会编译成为一个库, 这个库通过JNI提供的一个Stable的ABI 调用linux kernel;ABI是二进制程序接口 application binary interface.
[![img](https://ws2.sinaimg.cn/large/006tKfTcly1fo82ym8zujj30fe0ait99.jpg)](https://ws2.sinaimg.cn/large/006tKfTcly1fo82ym8zujj30fe0ait99.jpg)

# JNI详解

## JNIEnv详解

- JNIEnv作用 : JNIEnv 是一个指针,指向了一组JNI函数, 这些函数可以在jni.h中查询到,通过这些函数可以实现 Java层 与 JNI层的交互 , 通过JNIEnv 调用JNI函数 可以访问java虚拟机, 操作java对象;
  [![img](https://ws1.sinaimg.cn/large/006tKfTcly1fo83l5gywfj30fb0b7myf.jpg)](https://ws1.sinaimg.cn/large/006tKfTcly1fo83l5gywfj30fb0b7myf.jpg)

- JNI线程相关性 : JNIEnv只在当前的线程有效,JNIEnv不能跨线程传递, 相同的Java线程调用本地方法, 所使用的JNIEnv是相同的, 一个Native方法不能被不同的Java线程调用;

- JNIEnv结构体系 : JNIEnv指针指向一个线程相关的结构,线程相关结构指向一个指针数组,指针数组中的每个元素最终指向一个JNI函数.

  ### JNIEnv的C/C++声明

**jni.h中声明JNIEnv**:C语言中定义的JNIEnv 是 JNINativeInterface* , C++中定义的JNIEnv 是 _JNIEnv;

```
struct _JNIEnv;  
struct _JavaVM;  
typedef const struct JNINativeInterface* C_JNIEnv;  
  
#if defined(__cplusplus)    //为了兼容C 和 C++两种代码 使用该 宏加以区分  
typedef _JNIEnv JNIEnv;     //C++ 中的JNIEnv类型  
typedef _JavaVM JavaVM;  
#else  
typedef const struct JNINativeInterface* JNIEnv;//C语言中的JNIEnv类型  
typedef const struct JNIInvokeInterface* JavaVM;  
#endif
```

### C语言中的JNIEnv

关于JNIEnv指针调用解析 : C中JNIEnv就是 `const struct JNINativeInterface*`, `JNIEnv * env` 等价于 `JNINativeInterface** env`, 因此要得到JNINativeInterface结构体中定义的函数指针, 就必须先获取到 JNINativeInterface的一级指针对象 即 *env , 该一级指针对象就是 `JNINativeInterface* `, 然后通过该一级指针对象调用JNI函数 :`(*env)->NewStringUTF(env, “hello”)`;
在JNINativeInterface结构体中定义了一系列的关于Java操作的相关方法 :

```
/* 
 * Table of interface function pointers. 
 */  
struct JNINativeInterface {  
    void*       reserved0;  
    void*       reserved1;  
      
    ... ...  
      
    jboolean    (*CallStaticBooleanMethodV)(JNIEnv*, jclass, jmethodID,  
                        va_list);  
    jboolean    (*CallStaticBooleanMethodA)(JNIEnv*, jclass, jmethodID,  
                        jvalue*);  
    jbyte       (*CallStaticByteMethod)(JNIEnv*, jclass, jmethodID, ...);  
    jbyte       (*CallStaticByteMethodV)(JNIEnv*, jclass, jmethodID, va_list);  
      
    ... ...  
      
    void*       (*GetDirectBufferAddress)(JNIEnv*, jobject);  
    jlong       (*GetDirectBufferCapacity)(JNIEnv*, jobject);  
  
    /* added in JNI 1.6 */  
    jobjectRefType (*GetObjectRefType)(JNIEnv*, jobject);  
};
```

### C++中的JNIEnv

C++ 中的JNIEnv: C++ 中的JNIEnv 就是 _JNIEnv 结构体, 二者是等同的; 因此在调用 JNI函数的时候, 只需要使用 env->NewStringUTF(env, “hello”)方法即可, 不用在进行*运算;

```
/* 
 * C++ object wrapper. 
 * 
 * This is usually overlaid on a C struct whose first element is a 
 * JNINativeInterface*.  We rely somewhat on compiler behavior. 
 */  
struct _JNIEnv {  
    /* do not rename this; it does not seem to be entirely opaque */  
    const struct JNINativeInterface* functions;  
  
#if defined(__cplusplus)  
  
    jint GetVersion()  
    { return functions->GetVersion(this); }  
  
    jlong GetDirectBufferCapacity(jobject buf)  
    { return functions->GetDirectBufferCapacity(this, buf); }  
  
    /* added in JNI 1.6 */  
    jobjectRefType GetObjectRefType(jobject obj)  
    { return functions->GetObjectRefType(this, obj); }  
#endif /*__cplusplus*/  
};
```

## JNI方法命名规则(标准JNI规范)

JNI实现的方法 与 Java中Native方法的映射关系 : 使用方法名进行映射,可以使用 javah 工具进入 bin/classes 目录下执行命令, 即可生成头文件;
例如
java

```
package com.jni.demo;
public class JNIDemo {
	//定义一个本地方法
	public native void sayHello();
	public static void main(String[] args){
		//调用动态链接库
		System.loadLibrary("JNIDemo");
		JNIDemo jniDemo = new JNIDemo();
		jniDemo.sayHello();
	}
}
```

c++

```
#include<iostream.h>
#include "com_jni_demo_JNIDemo.h"

JNIEXPORT void JNICALL Java_com_jni_demo_JNIDemo_sayHello (JNIEnv * env, jobject obj)
{
cout<<"Hello World"<<endl;
}
```

JNI方法参数介绍:

- 参数① : 第一个参数是JNI接口指针 JNIEnv;
- 参数② : 如果Native方法是非静态的, 那么第二个参数就是对Java对象的引用, 如果Native方法是静态的, 那么第二个参数就是对Java类的Class对象的引用;

JNI方法名规范 : 返回值 + Java前缀 + 全路径类名 + 方法名 + 参数① JNIEnv + 参数② jobject + 其它参数;
*注意分隔符 : Java前缀 与 类名 以及类名之间的包名 和 方法名之间 使用 “_” 进行分割;*

声明**非静态**方法:

- Native方法 : public int hello (String str, int i);
- JNI方法: jint Java_shuliang_han_Hello_hello(JNIEnv * env, jobject obj, jstring str, jint i);

声明**静态**方法 :

- Native方法 : public static int hello (String str, int i);
- JNI方法 : jint Java_shuliang_han_Hello_hello(JNIEnv * env, jobject clazz, jstring str, jint i);

两种规范 : 以上是Java的标准JNI规范, 在Android中还有一套自定义的规范, 该规范是Android应用框架层 和 框架层交互使用的JNI规范, 依靠方法注册 映射 Native方法 和 JNI方法;

## JNI方法签名规则

JNI识别Java方法 : JNI依靠函数名 和 方法签名 识别方法, 函数名是不能唯一识别一个方法的, 因为方法可以重载, 类型签名代表了 参数 和 返回值;

- 签名规则 : (参数1类型签名 参数2类型签名 参数3类型签名 参数N类型签名…) 返回值类型签名

**注意参数列表中没有任何间隔**;

- Java类型 与 类型签名对照表 : 注意 boolean 与 long 不是大写首字母, 分别是 Z 与 J, 类是L全限定类名, 数组是[元素类型签名;
- 类的签名规则 :L + 全限定名 + ;三部分, 全限定类名以 / 分割;

Java类型 类型签名
boolean Z
byte B
char C
short S
int I
long J
float F
double D
类 L全限定类名;
数组 [元素类型签名
eg. long function(int n, String str, int[] arr);
该方法的签名 :(ILjava/lang/String;[I)J

方法签名介绍 :

```
public void helloFromJava(){  
        System.out.println("hello from java");  
    }  
      
    //C调用java中的带两个int参数的方法  
    public int Add(int x,int y){  
        return x + y;  
    }  
      
    //C调用java中参数为string的方法  
    public void printString(String s){  
        System.out.println(s);  
    }
```

- 返回值null, 参数null : void helloFromJava() 方法的签名是 “()V”, 括号里什么都没有代表参数为null, V代表返回值是void;
- 返回值int, 参数两个int : int Add(int x,int y) 方法的签名是 “(II)I”, 括号中II表示两个int类型参数, 右边括号外的I代表返回值是int类型;
- 返回值null, 参数String : void printString(String s) 方法签名是 “(Ljava/lang/String;)V”, 括号中的Ljava/lang/String; 表示参数是String类型, V表示返回值是void;

## Java中调用JNI

### JNI数据类型

[![img](https://ws1.sinaimg.cn/large/006tKfTcly1fo89hw3cstj30pw0cewfe.jpg)](https://ws1.sinaimg.cn/large/006tKfTcly1fo89hw3cstj30pw0cewfe.jpg)
数据类型表示方法 : int数组类型 jintArray , boolean数组 jbooleanArray …
头文件定义类型 : 这些基本的数据类型在[jni.h](https://android.googlesource.com/platform/libnativehelper/+/brillo-m9-dev/include/nativehelper/jni.h) 中都有相应的定义 :

```
/* Primitive types that match up with Java equivalents. */
typedef uint8_t  jboolean; /* unsigned 8 bits */
typedef int8_t   jbyte;    /* signed 8 bits */
typedef uint16_t jchar;    /* unsigned 16 bits */
typedef int16_t  jshort;   /* signed 16 bits */
typedef int32_t  jint;     /* signed 32 bits */
typedef int64_t  jlong;    /* signed 64 bits */
typedef float    jfloat;   /* 32-bit IEEE 754 */
typedef double   jdouble;  /* 64-bit IEEE 754 */
/* "cardinal indices and sizes" */
typedef jint     jsize;
#ifdef __cplusplus
/*
 * Reference types, in C++
 */
class _jobject {};
class _jclass : public _jobject {};
class _jstring : public _jobject {};
class _jarray : public _jobject {};
class _jobjectArray : public _jarray {};
class _jbooleanArray : public _jarray {};
class _jbyteArray : public _jarray {};
class _jcharArray : public _jarray {};
class _jshortArray : public _jarray {};
class _jintArray : public _jarray {};
class _jlongArray : public _jarray {};
class _jfloatArray : public _jarray {};
class _jdoubleArray : public _jarray {};
class _jthrowable : public _jobject {};
typedef _jobject*       jobject;
typedef _jclass*        jclass;
typedef _jstring*       jstring;
typedef _jarray*        jarray;
typedef _jobjectArray*  jobjectArray;
typedef _jbooleanArray* jbooleanArray;
typedef _jbyteArray*    jbyteArray;
typedef _jcharArray*    jcharArray;
typedef _jshortArray*   jshortArray;
typedef _jintArray*     jintArray;
typedef _jlongArray*    jlongArray;
typedef _jfloatArray*   jfloatArray;
typedef _jdoubleArray*  jdoubleArray;
typedef _jthrowable*    jthrowable;
typedef _jobject*       jweak;
#else /* not __cplusplus */
/*
 * Reference types, in C.
 */
typedef void*           jobject;
typedef jobject         jclass;
typedef jobject         jstring;
typedef jobject         jarray;
typedef jarray          jobjectArray;
typedef jarray          jbooleanArray;
typedef jarray          jbyteArray;
typedef jarray          jcharArray;
typedef jarray          jshortArray;
typedef jarray          jintArray;
typedef jarray          jlongArray;
typedef jarray          jfloatArray;
typedef jarray          jdoubleArray;
typedef jobject         jthrowable;
typedef jobject         jweak;
#endif /* not __cplusplus */
struct _jfieldID;                       /* opaque structure */
typedef struct _jfieldID* jfieldID;     /* field IDs */
struct _jmethodID;                      /* opaque structure */
typedef struct _jmethodID* jmethodID;   /* method IDs */
```

### JNI在Java和C语言之间传递int类型

Java中定义的方法 :

```
//将Java中的两个int值 传给C语言, 进行相加后, 返回java语言
//shuliang.han.ndkparameterpassing.DataProvider  
public native int add(int x, int y);
```

java中调用

```
case R.id.add:  
    int result = dataProvider.add(1, 2);  
    Toast.makeText(getApplicationContext(), "the add result : " + result, Toast.LENGTH_LONG).show();  
    break;
```

C语言中定义的方法 :

```
#include <jni.h>  
  
//方法签名, Java环境 和 调用native方法的类 必不可少, 后面的参数就是native方法的参数  
jint Java_shuliang_han_ndkparameterpassing_DataProvider_add(JNIEnv * env, jobject obj, jint x, jint y)  
{  
    return x + y;  
}
```

### 数组参数处理

- 获取数组长度方法 : jni中定义 - `jsize (*GetArrayLength)(JNIEnv*, jarray)`;

- 创建数组的相关方法

  ```
  jbooleanArray (*NewBooleanArray)(JNIEnv*, jsize);    
  jbyteArray    (*NewByteArray)(JNIEnv*, jsize);    
  jcharArray    (*NewCharArray)(JNIEnv*, jsize);    
  jshortArray   (*NewShortArray)(JNIEnv*, jsize);    
  jintArray     (*NewIntArray)(JNIEnv*, jsize);    
  jlongArray    (*NewLongArray)(JNIEnv*, jsize);    
  jfloatArray   (*NewFloatArray)(JNIEnv*, jsize);    
  jdoubleArray  (*NewDoubleArray)(JNIEnv*, jsize);
  ```

- 获取数组元素相关方法 :

  ```
  jboolean*   (*GetBooleanArrayElements)(JNIEnv*, jbooleanArray, jboolean*);    
  jbyte*      (*GetByteArrayElements)(JNIEnv*, jbyteArray, jboolean*);    
  jchar*      (*GetCharArrayElements)(JNIEnv*, jcharArray, jboolean*);    
  jshort*     (*GetShortArrayElements)(JNIEnv*, jshortArray, jboolean*);    
  jint*       (*GetIntArrayElements)(JNIEnv*, jintArray, jboolean*);    
  jlong*      (*GetLongArrayElements)(JNIEnv*, jlongArray, jboolean*);    
  jfloat*     (*GetFloatArrayElements)(JNIEnv*, jfloatArray, jboolean*);    
  jdouble*    (*GetDoubleArrayElements)(JNIEnv*, jdoubleArray, jboolean*);
  ```

**example**
c

```
jintArray Java_shuliang_han_ndkparameterpassing_DataProvider_intMethod(JNIEnv *env, jobject obj, jintArray arr)    
{    
    //获取arr大小    
    int len = (*env)->GetArrayLength(env, arr);    
        
    //在LogCat中打印出arr的大小    
    LOGI("the length of array is %d", len);    
        
    //如果长度为0, 返回arr    
    if(len == 0)    
        return arr;    
            
    //如果长度大于0, 那么获取数组中的每个元素    
    jint* p = (*env)->GetIntArrayElements(env, arr, 0);    
        
    //打印出数组中每个元素的值    
    int i = 0;    
    for(; i < len; i ++)    
    {    
        LOGI("arr[%d] = %d", i, *(p + i));    
    }    
        
    return arr;    
        
}
```

java

```
case R.id.intMethod:    
    int[] array = {1, 2, 3, 4, 5};    
    dataProvider.intMethod(array);    
    break;
```

## C代码回调Java方法或者使用java类的字段

C语言回调Java方法场景 :

- 复用方法 : 使用Java对象, 复用Java中的方法;
- 激活Java : C程序后台运行, 该后台程序一直运行, 某个时间出发后需要启动Java服务, 激活Android中的某个界面, 例如使用Intent启动一个Activity;

### 1 找到java对应的Class

为了能够在C/C++中使用Java类，jni.h头文件中专门定义了jclass类型来表示Java中的Class类
JNIEnv类中有如下几个简单的函数可以取得jclass:

- `jclass FindClass(const char* clsName)`:通过类的名称(类的全名，这时候包名不是用.号，而是用/来区分的)来获取jclass
  如: `jclass str = env->FindClass(“java/lang/String”)`;获取Java中的String对象的class对象。
- `jclass GetObjectClass(jobject obj)`:通过对象实例来获取jclass，相当于java中的getClass方法
- `jclass GetSuperClass(jclass obj)`:通过jclass可以获取其父类的jclass对象

*在C/C++本地代码中访问Java端的代码，一个常见的应用就是获取类的属性和调用类的方法，为了在C/C++中表示属性和方法，JNI在jni.h头文件中定义了jfieldId,jmethodID类型来分别代表Java端的属性和方法*

### 2.1 找到要使用的java字段的fieledID和使用

使用JNIEnv的：
GetFieldID/GetMethodID
GetStaticFieldID/GetStaticMethodID
来取得相应的jfieldID和jmethodID

下面来具体看一下这几个方法：

```
GetFieldID(jclass clazz,const char* name,const char* sign)
```

方法的参数说明:

clazz:这个简单就是这个字段依赖的类对象的class对象

name:这个是这个字段的名称

sign:这个是这个字段的签名(我们知道每个变量，每个方法都是有签名的)

**例子：在Java代码中定义一个属性，然后再C++代码中将其设置成另外的值，并且输出来**
java

```
package com.jni.demo;
public class JNIDemo {

    public int number = 0;//定义一个属性

//定义一个本地方法
    public native void sayHello();
    public static void main(String[] args){
        //调用动态链接库
        System.loadLibrary("JNIDemo");
        JNIDemo jniDemo = new JNIDemo();
        jniDemo.sayHello();
        System.out.print(jniDemo.number);
    }
}
```

c++

```
#include<iostream.h>
#include "com_jni_demo_JNIDemo.h"

JNIEXPORT void JNICALL Java_com_jni_demo_JNIDemo_sayHello (JNIEnv * env, jobject obj)
{
    //获取obj中对象的class对象
    jclass clazz = env->GetObjectClass(obj);
    //获取Java中的number字段的id(最后一个参数是number的签名)
    jfieldID id_number = env->GetFieldID(clazz,"number","I");
    //获取number的值
    jint number = env->GetIntField(obj,id_number);
    //输出到控制台
    cout<<number<<endl;
    //修改number的值为100,这里要注意的是jint对应C++是long类型,所以后面要加一个L
    env->SetIntField(obj,id_number,100L);
}
```

### 2.2 找到要调用的方法的methodID

```
GetMethodID(jclass clazz,const char* name,const char* sign)
```

方法的参数说明:

clazz:这个简单就是这个方法依赖的类对象的class对象

name:这个是这个方法的名称

sign:这个是这个方法的签名(我们知道每个变量，每个方法都是有签名的)

### 3 在C语言中调用相应方法

JNIEnv提供了众多的CallMethod和CallStaticMethod，还有CallNonvirtualMethod函数， 需要通过GetMethodID取得相应方法的jmethodID来传入到上述函数的参数中

调用示例方法的三种形式:

- CallMethod(jobject obj,jmethodID id,….);第一种是最常用的方式
- CallMethod(jobject obj,jmethodID id,va_list lst);第二种是当调用这个函数的时候有一个指向参数表的va_list变量时使用的(很少使用)
- CallMethod(jobject obj,jmethodID id,jvalue* v);第三种是当调用这个函数的时候有一个指向jvalue或jvalue数组的指针时用的
  jvalue在jni.h头文件中定义是一个union联合体，在C/C++中，我们知道union是可以存放不同类型的值，但是当你给其中一个类型赋值之后，这个union就是这种类型了，比如你给jvalue中的s赋值的话， jvalue就变成了jshort类型了，所以我们可以定义一个jvalue数组(这样就可以包含多种类型的参数了)传递到方法中。
  [![img](https://ws2.sinaimg.cn/large/006tKfTcly1fo8b17xs19j305r05bwfa.jpg)](https://ws2.sinaimg.cn/large/006tKfTcly1fo8b17xs19j305r05bwfa.jpg)

假如现在Java中有这样的一个方法:

boolean function(int a,double b,char c)

{

……..

}
(1) 在C++中使用第一种方式调用function方法:

```
env->CallBooleanMethod(obj , id_function , 10L, 3.4 , L’a’)
```

id_function是方法function的id;可以通过GetMethodID()方法获取

然后就是对应的参数，这个和Java中的可变参数类似，对于最后一个char类型的参数L’a’,为什么前面要加一个L,原因是Java中的字符时Unicode双字节的，而C++中的字符时单字节的，所以要变成宽字符，前面加一个L

(2) 在C++中使用第三种方式调用function方法:

```
jvalue* args = new jvalue[3];//定义jvalue数组

args[0].i = 10L;//i是jvalue中的jint值

args[1].d = 3.44;

args[2].c = L’a’;

env->CallBooleanMethod(obj, id_function, args);

delete[] args;//释放指针堆内存
```

**例子:C++中调用Java中的方法:**

Java代码：

```
public double max(double value1,double value2){
    return value1>value2 ? value1:value2;
}
```

在C++中的代码:

```
JNIEXPORT void JNICALL Java_com_jni_demo_JNIDemo_sayHello (JNIEnv * env, jobject obj)
{
    //获取obj中对象的class对象
    jclass clazz = env->GetObjectClass(obj);
    //获取Java中的max方法的id(最后一个参数是max方法的签名)
    jmethodID id_max = env->GetMethodID(clazz,"max","(DD)D");
    //调用max方法
    jdouble doubles = env->CallDoubleMethod(obj,id_max,1.2,3.4);
    //输出返回值
    cout<<doubles<<endl;
}
```

# 参考链接

https://github.com/eternalsakura/ctf_pwn/tree/master/android逆向/Log日志分析源码
http://blog.csdn.net/shulianghan/article/details/18964835
http://www.wjdiankong.cn/java中jni的使用详解第一篇helloworld/
http://www.wjdiankong.cn/java中jni的使用详解第二篇jnienv类型和jobject类型的解释/
http://www.wjdiankong.cn/java中jni的使用详解第三篇jnienv类型中方法的使用/
https://android.googlesource.com/platform/libnativehelper/+/brillo-m9-dev/include/nativehelper/jni.h
https://github.com/han1202012/NDKParameterPassing
https://github.com/han1202012/NDK_Callback/blob/master/jni/jni.c

[# android逆向基础](https://eternalsakura13.com/tags/android逆向基础/)