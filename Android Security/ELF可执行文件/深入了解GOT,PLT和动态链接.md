# 深入了解GOT,PLT和动态链接

src:https://evilpan.com/2018/04/09/about-got-plt/


![在这里插入图片描述](images/20200906191512981.png)
之前几篇介绍exploit的文章, 有提到`return-to-plt`的技术. 当时只简单介绍了 GOT和PLT表的基本作用和他们之间的关系, 所以今天就来详细分析下其具体的工作过程.

>本文所用的依然是Linux x86 64位环境, 不过分析的ELF文件是32位的(-m32).


# 大局观
首先, 我们要知道, GOT和PLT只是一种重定向的实现方式. 所以为了理解他们的作用, 就要先知道什么是重定向, 以及我们为什么需要重定向.

`重定向(relocations)`, 简单来说就是二进制文件中留下的”坑”, 预留给外部变量或函数. 这里的变量和函数统称为`符号(symbols)`. 在编译期我们通常只知道外部符号的类型 (变量类型和函数原型), 而不需要知道具体的值(变量值和函数实现). 而这些预留的”坑”, 会在用到之前(链接期间或者运行期间)填上. 在链接期间填上主要通过工具链中的连接器, 比如GNU链接器ld; 在运行期间填上则通过动态连接器, 或者说解释器(interpreter)来实现. 比如:

```c
$ file /bin/ls
/bin/ls: ELF 64-bit LSB shared object, x86-64, version 1 (SYSV), dynamically linked,
interpreter /lib64/ld-linux-x86-64.so.2, for GNU/Linux 2.6.32, BuildID[sha1]=3c233e12c466a83aa9b2094b07dbfaa5bd10eccd, stripped
```

可以看到`/bin/ls`的解释器是`/lib64/ld-linux-x86-64.so.2`.

在本文中, 用下面两个简单的c文件来进行说明, 首先是symbol.c, 定义了一个函数变量:

```c
// symbol.c
int my_var = 42;
int my_func(int a, int b) {
    return a + b;
}
```
编译为动态链接库:

```c
gcc -g -m32 -masm=intel -shared -fPIC symbol.c -o libsymbol.so
```

另一个文件是main.c, 调用该动态链接库:

```c
// main.c
int var = 10;
extern int my_var;
extern int my_func(int, int);

int main() {
    int a, b;
    a = var;
    b = my_var;
    return my_func(a, b);
}
```

分别编译两个版本, 位置相关的main和位置无关的main_pi, 具体会稍后解释.

```c
# 位置相关
gcc -g -m32 -masm=intel -L. -lsymbol -no-pie -fno-pic main.c libsymbol.so -o main
# 位置无关
gcc -g -m32 -masm=intel -L. -lsymbol main.c libsymbol.so -o main_pi
```

## 符号表
函数和变量作为符号被存在可执行文件中, 不同类型的符号又聚合在一起, 称为符号表. 有两种类型的符号表, 一种是常规的(.symtab和.strtab), 另一种是动态的(.dynsym和.dynstr), 他们都在对应的[section](https://docs.oracle.com/cd/E19120-01/open.solaris/819-0690/6n33n7fda/index.html)中, 以main为例:

```c
$ readelf -S ./main
  [Nr] Name              Type            Addr     Off    Size   ES Flg Lk Inf Al
  [ 5] .dynsym           DYNSYM          080481ec 0001ec 0000b0 10   A  6   1  4
  [ 6] .dynstr           STRTAB          0804829c 00029c 000085 00   A  0   0  1
...
  [33] .symtab           SYMTAB          00000000 00120c 000490 10     34  52  4
  [34] .strtab           STRTAB          00000000 00169c 0001e1 00      0   0  1
```

常规的符号表通常只在调试时用到. 我们平时用的strip命令删除的就是该符号表; 而动态符号表则是程序执行时候真正会查找的目标.

## 位置无关代码
刚刚编译动态链接库时指定了-fPIC, 编译main_pi时(默认)指定了-pie, 其实都是为了 生成位置无关的代码, 那么什么是位置无关? 为什么要位置无关?

我们执行一个可执行文件的时候, 其实是先将磁盘上的该文件读取到内存中, 然后再执行. 而每个进程都有自己的虚拟内存空间, 以32位程序为例, 就有2^32=4GB的寻址空间, 从0x00000000 到0xffffffff. 这里暂时不深入介绍, 只需要知道虚拟内存最终会通过页表映射到物理内存中.

>当然, 如果你感兴趣, 强烈推荐你去看下[Gustavo Duarte的这篇文章](https://manybutfinite.com/post/anatomy-of-a-program-in-memory/).

按照链接器的约定, 32位程序会加载到0x08048000这个地址中([为什么?](http://www.iecc.com/linker/linker04.html)), 所以我们写程序时, 可以以这个地址为基础, 对变量进行绝对地址寻址. 以main为例:

```c
$ readelf -S ./main | grep "\.data"
  [Nr] Name              Type            Addr     Off    Size   ES Flg Lk Inf Al
  [25] .data             PROGBITS        0804a014 001014 00000c 00  WA  0   0  4
```
.data部分在可执行文件中的偏移量为0x1014, 那么加载到虚拟内存中的地址应该是 0x8048000+0x1014=0x804a14, 正好和显示的结果一样. 再看看main函数的汇编代码:

```c
$ objdump  -d ./main | grep "<main>" -A 15
080484db <main>:
 80484db:	8d 4c 24 04          	lea    ecx,[esp+0x4]
 80484df:	83 e4 f0             	and    esp,0xfffffff0
 80484e2:	ff 71 fc             	push   DWORD PTR [ecx-0x4]
 80484e5:	55                   	push   ebp
 80484e6:	89 e5                	mov    ebp,esp
 80484e8:	51                   	push   ecx
 80484e9:	83 ec 14             	sub    esp,0x14
 80484ec:	a1 1c a0 04 08       	mov    eax,ds:0x804a01c
 80484f1:	89 45 f4             	mov    DWORD PTR [ebp-0xc],eax
 80484f4:	a1 20 a0 04 08       	mov    eax,ds:0x804a020
 80484f9:	89 45 f0             	mov    DWORD PTR [ebp-0x10],eax
 80484fc:	83 ec 08             	sub    esp,0x8
 80484ff:	ff 75 f0             	push   DWORD PTR [ebp-0x10]
 8048502:	ff 75 f4             	push   DWORD PTR [ebp-0xc]
 8048505:	e8 a6 fe ff ff       	call   80483b0 <my_func@plt>

```
注意80484ec这行, 可以看到获取变量直接用的绝对地址0x804a01c(正好在.data范围内). 用gdb(在启动程序之前)可看到该地址正是`var`变量的地址, 且初始值为10:

```c
$ gdb ./main
(gdb) x/xw 0x804a01c
0x804a01c <var>:	0x0000000a
```
按绝对地址寻址, 对可执行文件来说不是什么大问题, 因为一个进程只有一个主函数. 可对于动态链接库而言就比较麻烦, 如果每个.so文件都要求加载到某个绝对地址, 那简直是个噩梦, 因为你无法保证不和别人的.so加载地址冲突. 所以就有了位置无关代码的概念. 以位置无关的方式编译的main_pi, 来看看其相关信息:

```c
$ readelf -S ./main_pi | grep "\.data"
  [Nr] Name              Type            Addr     Off    Size   ES Flg Lk Inf Al
  [25] .data             PROGBITS        00002014 001014 00000c 00  WA  0   0  4
```
偏移量还是固定的, 但Addr部分不再是绝对地址. 也就是说程序可以加载到虚拟内存的任意位置. 听起来很神奇? 其实实现很简单, 继续看看main()的汇编:

```c
$ objdump -d main_pi | grep "<main>" -A 20
00000660 <main>:
 660:	8d 4c 24 04          	lea    ecx,[esp+0x4]
 664:	83 e4 f0             	and    esp,0xfffffff0
 667:	ff 71 fc             	push   DWORD PTR [ecx-0x4]
 66a:	55                   	push   ebp
 66b:	89 e5                	mov    ebp,esp
 66d:	53                   	push   ebx
 66e:	51                   	push   ecx
 66f:	83 ec 10             	sub    esp,0x10
 672:	e8 36 00 00 00       	call   6ad <__x86.get_pc_thunk.ax>
 677:	05 89 19 00 00       	add    eax,0x1989
 67c:	8b 90 1c 00 00 00    	mov    edx,DWORD PTR [eax+0x1c]
 682:	89 55 f4             	mov    DWORD PTR [ebp-0xc],edx
 685:	8b 90 f0 ff ff ff    	mov    edx,DWORD PTR [eax-0x10]
 68b:	8b 12                	mov    edx,DWORD PTR [edx]
 68d:	89 55 f0             	mov    DWORD PTR [ebp-0x10],edx
 690:	83 ec 08             	sub    esp,0x8
 693:	ff 75 f0             	push   DWORD PTR [ebp-0x10]
 696:	ff 75 f4             	push   DWORD PTR [ebp-0xc]
 699:	89 c3                	mov    ebx,eax
 69b:	e8 20 fe ff ff       	call   4c0 <my_func@plt>
```
注意67c~682处, 和之前的区别是这次通过eax寄存器来对变量进行寻址, 不过有个__x86.get_pc_thunk.ax函数, 其作用很简单, 在之前的[IOLI-crackme0x06-0x09 writeup](https://evilpan.com/2018/03/10/ioli-crackme6-9/#crackme0x09)中有简单介绍过:

```c
objdump -d main_pi | grep "__x86.get_pc_thunk.ax" -A 2
000006ad <__x86.get_pc_thunk.ax>:
 6ad:	8b 04 24             	mov    eax,DWORD PTR [esp]
 6b0:	c3                   	ret
```

作用就是把esp(即返回地址)的值保存在eax(PIC寄存器)中, 在接下来寻址用. 有人可能好奇, 为什么这么麻烦, 直接用eip寄存器不就行了? 其实64位下就是这样操作的! 不过32位下不支持直接访问PC寄存器, 所以就多了一层间接的函数调用.

扯远了, 经过672和677两条指令后, eax的值将等于相对当前PC指针的固定位移. 只看静态代码的话, 可知eax=0x677+0x1989=0x2000, 而这个地址是…

```c
$ readelf -S ./main_pi | grep 2000 -C 1
  [23] .got              PROGBITS        00001fe4 000fe4 00001c 04  WA  0   0  4
  [24] .got.plt          PROGBITS        00002000 001000 000014 04  WA  0   0  4
  [25] .data             PROGBITS        00002014 001014 00000c 00  WA  0   0  4
```

.got.plt的起始地址! 这个section我们接下来会说到. 现在先看汇编的67c处, 通过eax+0x1c=0x201c获取了变量的值, 这个地址已经进入到了.data之中:

```c
$ gdb ./main_pi 
(gdb) x/xw 0x2000+0x1c
0x201c <var>:	0x0000000a
```

所以, 位置无关代码实际上就是通过运行时PC指针的值来找到代码所引用的 其他符号的位置, 不管二进制文件被加载到哪个位置, 都可以正确执行.

### 缺点
位置无关代码的缺点是, 在执行时要保留一个寄存器作为PIC寄存器, 有可能会导致寄存器不够用; 还有一个缺点是运行时要经过计算来获得 符号的地址, 从某种方面来说也对运行速度有点小影响.

### 优点
位置无关代码的优点就跟他名字一样, 可以保证加载到任意地址都能 正常执行, 这也是每个动态链接库都需要支持的.

# 动态链接
刚刚我们说位置无关代码的时候有看到, PIC寄存器为.got.plt的地址, 然后按偏移量 来获取变量. 上面只看了eax+0x1c即从.data段获取的内容(var), 还有一个参数是通过 eax-0x10即.got段之中获取的my_var. 后者是在symbol.c中定义的, 所以其内容在编译期 未知. 如果是静态链接, 则可以在链接时解析符号的值. 我们这里主要考虑动态链接的情况.

## 一些定义
上面说了很多.got, .plt啥的, 那么这些section到底是做什么用的呢. 其实这些都是 链接器(或解释器, 下面统称为链接器)在执行重定向时会用到的部分, 先来看他们的定义.

### .got && .got.plt
我们常说的GOT, 即Global Offset Table, 全局偏移表, 包括了`.got`和`.got.plt. `前者是全局变量，后者是全局函数。

.got.plt相当于.plt的GOT全局偏移表, 其内容有两种情况, 1)如果在之前查找过该符号, 内容为外部函数的具体地址. 2)如果没查找过, 则内容为跳转回.plt的代码, 并执行查找. 至于为什么要这么绕, 后面会说明具体原因.

这是链接器在执行链接时实际上要填充的部分, 保存了所有外部符号的地址信息. 不过值得注意的是, 在i386架构下, 除了每个函数占用一个GOT表项外，GOT表项还保留了 3个公共表项, 每项32位(4字节), 保存在前三个位置, 分别是:

got[0]: 本ELF动态段(.dynamic段)的装载地址
got1: 本ELF的`link_map`数据结构描述符地址
got2:` _dl_runtime_resolve`函数的地址
其中,` link_map`数据结构的定义如下:

```c
struct link_map
{
   /* Shared library's load address. */
   ElfW(Addr) l_addr;                 
   /* Pointer to library's name in the string table. */                                 
   char *l_name;    
   /* 
        Dynamic section of the shared object.
        Includes dynamic linking info etc.
        Not interesting to us.  
   */                   
   ElfW(Dyn) *l_ld;   
   /* Pointer to previous and next link_map node. */                 
   struct link_map *l_next, *l_prev;   
};
```

### .plt
这也是我们常说的PLT, 即Procedure Linkage Table, 进程链接表. 这个表里包含了一些代码, 用来(1)调用链接器来解析某个外部函数的地址, 并填充到.got.plt中, 然后跳转到该函数; 或者 (2)直接在.got.plt中查找并跳转到对应外部函数(如果已经填充过).

### .plt.got
说实话, 这部分我还不知道有什么具体作用, 可能是为了对称吧. 逃)

对于我们将要研究的main程序, 这些段的地址如下:

```c
$ readelf -S main | egrep  '.plt|.got'
  [Nr] Name              Type            Addr     Off    Size   ES Flg Lk Inf Al
  [12] .plt              PROGBITS        080483a0 0003a0 000030 04  AX  0   0 16
  [13] .plt.got          PROGBITS        080483d0 0003d0 000008 00  AX  0   0  8
  [23] .got              PROGBITS        08049ffc 000ffc 000004 04  WA  0   0  4
  [24] .got.plt          PROGBITS        0804a000 001000 000014 04  WA  0   0  4
```
## 变量
有了上面的定义, 先看变量的解析过程, 以main为例(位置相关的), 查看需要重定向的符号:

```c
$ readelf --relocs ./main

Relocation section '.rel.dyn' at offset 0x358 contains 2 entries:
 Offset     Info    Type            Sym.Value  Sym. Name
08049ffc  00000206 R_386_GLOB_DAT    00000000   __gmon_start__
0804a020  00000605 R_386_COPY        0804a020   my_var

Relocation section '.rel.plt' at offset 0x368 contains 2 entries:
 Offset     Info    Type            Sym.Value  Sym. Name
0804a00c  00000107 R_386_JUMP_SLOT   00000000   my_func
0804a010  00000307 R_386_JUMP_SLOT   00000000   __libc_start_main@GLIBC_2.0
```

`my_var`的地址为0804a020, 注意这里实际上是.bss段, 如下:

```c
$ readelf -S main | grep 0804a020 -B 2
  [24] .got.plt          PROGBITS        0804a000 001000 000014 04  WA  0   0  4
  [25] .data             PROGBITS        0804a014 001014 00000c 00  WA  0   0  4
  [26] .bss              NOBITS          0804a020 001020 000008 00  WA  0   0  4
```

因为main.c里只是声明变量而且没初始化, 在链接前并不知道是否在外部定义. 同时, 该变量的值一开始是不知道的, 我们可以通过gdb来验证:

```c
(gdb) x/dw 0x0804a020
0x804a020 <my_var>:	0
```

显示值为0, 但实际上在symbol.c中定义了其值为42, 启动前我们先在这里下个观察点, 看看究竟是什么时候加载进去的:

```c
(gdb) set environment LD_LIBRARY_PATH=.
(gdb) watch -l *0x804a020
Hardware watchpoint 1: -location *0x804a020
(gdb) run
Starting program: /home/pan/project/cFile/shared_library/plt/main 

Hardware watchpoint 1: -location *0x804a020

Old value = 0
New value = 42
0xf7ff2e08 in ?? () from /lib/ld-linux.so.2
(gdb) x/xd 0x804a020
0x804a020 <my_var>:	42
```

所以, 确实是链接器`/lib/ld-linux.so.2`负责填充了该变量的内容. 而且是在程序运行之前就完成了符号解析.

## 函数
接下来看看外部函数符号. 外部函数的内容(指令)也是像变量一样在 程序运行之前完成填充的吗? 其实这理论上是可以的, 事实上稍有不同.

### 静态分析
我们先从汇编看看main是如何调用my_func()函数的:

```c
(gdb) disassemble  main
Dump of assembler code for function main:
   0x080484db <+0>:	lea    ecx,[esp+0x4]
   0x080484df <+4>:	and    esp,0xfffffff0
   0x080484e2 <+7>:	push   DWORD PTR [ecx-0x4]
   0x080484e5 <+10>:	push   ebp
   0x080484e6 <+11>:	mov    ebp,esp
   0x080484e8 <+13>:	push   ecx
   0x080484e9 <+14>:	sub    esp,0x14
   0x080484ec <+17>:	mov    eax,ds:0x804a01c
   0x080484f1 <+22>:	mov    DWORD PTR [ebp-0xc],eax
   0x080484f4 <+25>:	mov    eax,ds:0x804a020
   0x080484f9 <+30>:	mov    DWORD PTR [ebp-0x10],eax
   0x080484fc <+33>:	sub    esp,0x8
   0x080484ff <+36>:	push   DWORD PTR [ebp-0x10]
   0x08048502 <+39>:	push   DWORD PTR [ebp-0xc]
   0x08048505 <+42>:	call   0x80483b0 <my_func@plt>
```

调用的地址是0x80483b0, 在.plt段中, 之前说了PLT的定义, 现在具体看看里面的内容:

```c
(gdb) disassemble 0x80483b0
Dump of assembler code for function my_func@plt:
   0x080483b0 <+0>:	jmp    DWORD PTR ds:0x804a00c
   0x080483b6 <+6>:	push   0x0
   0x080483bb <+11>:	jmp    0x80483a0
End of assembler dump.
```

首先是跳转到`*0x804a00c`, 该地址在.got.plt之中, 之前说了, .got.plt相当于 .plt的GOT, 而GOT本身相当于一个数组, 看看该”数组”的内容:

```c
(gdb) x/4xw 0x804a00c
0x804a00c:	0x080483b6	0x080483c6	0x00000000	0x00000000
```

所以, 0x080483b0这里的跳转, 相当于跳转到0x080483b6, 即下一条指令! 这个多余的跳转先打个问号, 把流程走完再说. 接着, 跳转到了0x80483a0, 这个地址, 是.plt的起始地址, 这里的指令如下:

```c
(gdb) x/2i 0x080483a0
   0x80483a0:	push   DWORD PTR ds:0x804a004
   0x80483a6:	jmp    DWORD PTR ds:0x804a008
```

跳转到了0x804a008, 在前面我们知道0x804a000是.got.plt的地址, 而在上一节的定义中, 也知道了.got表前三项的作用, 0x804a008 正好是第三项got2, 即`_dl_runtime_resolve`函数的地址. 0x804a004 则是调用该函数的参数, 且值为got1, 即本ELF的`link_map`的地址. 如下, 在进程未启动前, got1和got2都为0, 在启动时由链接器装填:

```c
(gdb) x/4xw 0x804a000
0x804a000:	0x08049f0c	0x00000000	0x00000000	0x080483b6
```

因此, 实际上(第一次)调用`my_func@plt`就相当于调用了 `_dl_runtime_resolve((link_map *)m, 0)`! 其中`link_map`提供了运行时的必要信息, 而0则是`my_func`函数的偏移(在`my_func@plt`中push 0x0). 该函数定义在[glibc/sysdeps/i386/dl-trampoline.S](https://code.woboq.org/userspace/glibc/sysdeps/i386/dl-trampoline.S.html)中, 关键代码如下:

```c
_dl_runtime_resolve:
        cfi_adjust_cfa_offset (8)
        pushl %eax                # Preserve registers otherwise clobbered.
        cfi_adjust_cfa_offset (4)
        pushl %ecx
        cfi_adjust_cfa_offset (4)
        pushl %edx
        cfi_adjust_cfa_offset (4)
        movl 16(%esp), %edx        # Copy args pushed by PLT in register.  Note
        movl 12(%esp), %eax        # that `fixup' takes its parameters in regs.
        call _dl_fixup                # Call resolver.
        popl %edx                # Get register content back.
        cfi_adjust_cfa_offset (-4)
        movl (%esp), %ecx
        movl %eax, (%esp)        # Store the function address.
        movl 4(%esp), %eax
        ret $12                        # Jump to function address.
```

从注释里也可以看出来, 该函数实际上做了两件事:

* 1)解析出my_func的地址并将值填入.got.plt中.
* 2)跳转执行真正的my_func函数.

### 动态分析
上面虽然用了gdb, 但程序并未运行, 只是分析静态的汇编代码, 为了验证上面的说法, 我们需要进行动态分析. 接着上面的分析, 我们这次在调用`_dl_runtime_resolve` 前打上断点. 还记得之前在`my_func@plt`中一次多余的跳转吗? 当时打了个问号, 现在就来解答这个疑问. 在0x804a00c处打上观察点并运行:

```c
(gdb) b *0x80483a6
(gdb) watch -l *0x804a00c
(gdb) run
Breakpoint 1, 0x080483a6 in ?? ()
(gdb) x/xw 0x804a00c
0x804a00c:	0x080483b6
(gdb) continue
Hardware watchpoint 1: -location *0x804a00c

Old value = 0x80483b6
New value = 0xf7fcf4f0
0xf7fe8113 in ?? () from /lib/ld-linux.so.2
(gdb) disassemble 0xf7fcf4f0
Dump of assembler code for function my_func:
...
```

可以看到, 在`_dl_runtime_resolve`之前, 0x804a00c地址的值为0x080483b6, 即下一条指令. 而运行之后, 该地址的值变为0xf7fcf4f0, 正是`my_func`的加载地址! 也就是说, `my_func`函数的地址是在第一次调用时, 才通过连接器动态解析并加载到 .got.plt中的. 而这个过程, 也称之为`延时加载`或者`惰性加载`.

注：在Android下，ELF的`.got.plt`的符号填充过程是在`linker64`加载和链接依赖库的过程中完成的，ELF的所有外部函数符号的真实地址会在依赖库加载完后全部设置完成，这意味Android系统的链接器没有使用延迟绑定的技术.

## 延时加载
延时加载的好处是, 只有当外部函数被调用了才会去进行动态加载, 降低程序的启动时间. 而第一次加载之后, 对于后续的调用就可以直接跳转而不需要再去加载. 这样一方面减少了进程的启动开销, 另一方面也不会造成太多额外的运行时开销, 所以延时加载在当今也是广泛应用的一个思想. 对于位置无关的代码, 延时加载的过程也是类似的, 并没有太大区别. 读者可以自己去追踪一下.

# 总结
为了灵活利用虚拟内存空间, 所以编译器可以产生位置无关的代码. 可执行文件可以是位置无关的, 也可以是位置相关的, 动态链接库 绝大多数都是位置无关的. GOT表可写不可执行, PLT可执行不可写, 他们相互作用来实现函数符号的延时绑定. ASLR并不随机化PLT部分, 所以对ret2plt攻击没有直接影响. 为防止恶意修改got, 链接器提供了RELRO 选项, 去除got的写权限, 但也牺牲了延时绑定带来的好处.

另外本文也详细介绍了return-to-dlresolve攻击的原理，以加深延时绑定的理解。 在32位情况下可以无需信息泄露来主动解析和调用外部动态库；但在64位环境中， 仍需要泄露link_map地址来防止段错误。