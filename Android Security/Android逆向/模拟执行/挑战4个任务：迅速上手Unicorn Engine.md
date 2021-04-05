# 挑战4个任务：迅速上手Unicorn Engine

## 前言

在这篇教程中，您将通过解决实际问题来练习如何使用Unicorn Engine。一共有4个练习，其中我将会详细讲解第一个练习，而对于其他练习我们会提供提示和解决方案供大家阅读。

### FAQ：

#### 1、什么是Unicorn Engine？

Unicore Engine是一个模拟器，尽管并不太常见。通过该模拟器，您不用模仿整个程序或系统。这一模拟器不支持系统调用，必须先映射内存，并手动将数据写入到内存中，然后才能从指定的地址开始模拟。

#### 2、这篇文章中的内容可以用于什么场景？

我们可以在不创建有害进程的前提下，从恶意软件中调用一个特定的函数。此外还可以用于CTF比赛，用于基于漏洞注入的自动软件测试，也可以用于能预测未来的gdb插件（例如实现进一步的跳转），还可以用来仿真混淆的代码。

#### 3、要开始本教程的练习，我需要安装什么？

需要安装Unicorn Engine，并连接Python。此外，还需要一个反汇编工具。

 

## 任务1

该任务来自hxp CTF 2017，名称为斐波那契，地址为：https://ctftime.org/event/489 。
二进制文件可以在这里下载：http://eternal.red/assets/files/2017/UE/fibonacci 。
当我们运行这个程序的时候，我们可以注意到，它会计算并打印我们的Flag，但这个过程非常缓慢，并且Flag的计算过程会随着字节的增多变得越来越慢。
该题的Flag为：hxp{F。
这就意味着，我们需要对程序进行优化，以在合理的时间内得到Flag。
在IDA Pro的帮助下，我们将代码反编译成像C语言一样的伪代码。尽管代码最终并不一定能被正确地反编译，但我们通过这一过程，可以对代码的具体功能有一定的了解。

```
__int64 __fastcall main(__int64 a1, char **a2, char **a3)
{
  void *v3; // rbp@1
  int v4; // ebx@1
  signed __int64 v5; // r8@2
  char v6; // r9@3
  __int64 v7; // r8@3
  char v8; // cl@3
  __int64 v9; // r9@5
  int a2a; // [sp+Ch] [bp-1Ch]@3

  v3 = &encrypted_flag;
  v4 = 0;
  setbuf(stdout, 0LL);
  printf("The flag is: ", 0LL);
  while ( 1 )
  {
    LODWORD(v5) = 0;
    do
    {
      a2a = 0;
      fibonacci(v4 + v5, &a2a);
      v8 = v7;
      v5 = v7 + 1;
    }
    while ( v5 != 8 );
    v4 += 8;
    if ( (unsigned __int8)(a2a << v8) == v6 )
      break;
    v3 = (char *)v3 + 1;
    _IO_putc((char)(v6 ^ ((_BYTE)a2a << v8)), stdout);
    v9 = *((char *)v3 - 1);
  }
  _IO_putc(10, stdout);
  return 0LL;
}

unsigned int __fastcall fibonacci(int i, _DWORD *a2)
{
  _DWORD *v2; // rbp@1
  unsigned int v3; // er12@3
  unsigned int result; // eax@3
  unsigned int v5; // edx@3
  unsigned int v6; // esi@3
  unsigned int v7; // edx@4

  v2 = a2;
  if ( i )
  {
    if ( i == 1 )
    {
      result = fibonacci(0, a2);
      v5 = result - ((result >> 1) & 0x55555555);
      v6 = ((result - ((result >> 1) & 0x55555555)) >> 2) & 0x33333333;
    }
    else
    {
      v3 = fibonacci(i - 2, a2);
      result = v3 + fibonacci(i - 1, a2);
      v5 = result - ((result >> 1) & 0x55555555);
      v6 = ((result - ((result >> 1) & 0x55555555)) >> 2) & 0x33333333;
    }
    v7 = v6 + (v5 & 0x33333333) + ((v6 + (v5 & 0x33333333)) >> 4);
    *v2 ^= ((BYTE1(v7) & 0xF) + (v7 & 0xF) + (unsigned __int8)((((v7 >> 8) & 0xF0F0F) + (v7 & 0xF0F0F0F)) >> 16)) & 1;
  }
  else
  {
    *a2 ^= 1u;
    result = 1;
  }
  return result;
}
```

下面是主函数的汇编代码：

```
.text:0x4004E0 main            proc near               ; DATA XREF: start+1Do
.text:0x4004E0
.text:0x4004E0 var_1C          = dword ptr -1Ch
.text:0x4004E0
.text:0x4004E0                 push    rbp
.text:0x4004E1                 push    rbx
.text:0x4004E2                 xor     esi, esi        ; buf
.text:0x4004E4                 mov     ebp, offset unk_4007E1
.text:0x4004E9                 xor     ebx, ebx
.text:0x4004EB                 sub     rsp, 18h
.text:0x4004EF                 mov     rdi, cs:stdout  ; stream
.text:0x4004F6                 call    _setbuf
.text:0x4004FB                 mov     edi, offset format ; "The flag is: "
.text:0x400500                 xor     eax, eax
.text:0x400502                 call    _printf
.text:0x400507                 mov     r9d, 49h
.text:0x40050D                 nop     dword ptr [rax]
.text:0x400510
.text:0x400510 loc_400510:                             ; CODE XREF: main+8Aj
.text:0x400510                 xor     r8d, r8d
.text:0x400513                 jmp     short loc_40051B
.text:0x400513 ; ---------------------------------------------------------------------------
.text:0x400515                 align 8
.text:0x400518
.text:0x400518 loc_400518:                             ; CODE XREF: main+67j
.text:0x400518                 mov     r9d, edi
.text:0x40051B
.text:0x40051B loc_40051B:                             ; CODE XREF: main+33j
.text:0x40051B                 lea     edi, [rbx+r8]
.text:0x40051F                 lea     rsi, [rsp+28h+var_1C]
.text:0x400524                 mov     [rsp+28h+var_1C], 0
.text:0x40052C                 call    fibonacci
.text:0x400531                 mov     edi, [rsp+28h+var_1C]
.text:0x400535                 mov     ecx, r8d
.text:0x400538                 add     r8, 1
.text:0x40053C                 shl     edi, cl
.text:0x40053E                 mov     eax, edi
.text:0x400540                 xor     edi, r9d
.text:0x400543                 cmp     r8, 8
.text:0x400547                 jnz     short loc_400518
.text:0x400549                 add     ebx, 8
.text:0x40054C                 cmp     al, r9b
.text:0x40054F                 mov     rsi, cs:stdout  ; fp
.text:0x400556                 jz      short loc_400570
.text:0x400558                 movsx   edi, dil        ; c
.text:0x40055C                 add     rbp, 1
.text:0x400560                 call    __IO_putc
.text:0x400565                 movzx   r9d, byte ptr [rbp-1]
.text:0x40056A                 jmp     short loc_400510
.text:0x40056A ; ---------------------------------------------------------------------------
.text:0x40056C                 align 10h
.text:0x400570
.text:0x400570 loc_400570:                             ; CODE XREF: main+76j
.text:0x400570                 mov     edi, 0Ah        ; c
.text:0x400575                 call    __IO_putc
.text:0x40057A                 add     rsp, 18h
.text:0x40057E                 xor     eax, eax
.text:0x400580                 pop     rbx
.text:0x400581                 pop     rbp
.text:0x400582                 retn
.text:0x400582 main            endp
```

fibonacci函数的汇编代码如下：

```
.text:0x400670 fibonacci       proc near               ; CODE XREF: main+4Cp
.text:0x400670                                         ; fibonacci+19p ...
.text:0x400670                 test    edi, edi
.text:0x400672                 push    r12
.text:0x400674                 push    rbp
.text:0x400675                 mov     rbp, rsi
.text:0x400678                 push    rbx
.text:0x400679                 jz      short loc_4006F8
.text:0x40067B                 cmp     edi, 1
.text:0x40067E                 mov     ebx, edi
.text:0x400680                 jz      loc_400710
.text:0x400686                 lea     edi, [rdi-2]
.text:0x400689                 call    fibonacci
.text:0x40068E                 lea     edi, [rbx-1]
.text:0x400691                 mov     r12d, eax
.text:0x400694                 mov     rsi, rbp
.text:0x400697                 call    fibonacci
.text:0x40069C                 add     eax, r12d
.text:0x40069F                 mov     edx, eax
.text:0x4006A1                 mov     ebx, eax
.text:0x4006A3                 shr     edx, 1
.text:0x4006A5                 and     edx, 55555555h
.text:0x4006AB                 sub     ebx, edx
.text:0x4006AD                 mov     ecx, ebx
.text:0x4006AF                 mov     edx, ebx
.text:0x4006B1                 shr     ecx, 2
.text:0x4006B4                 and     ecx, 33333333h
.text:0x4006BA                 mov     esi, ecx
.text:0x4006BC
.text:0x4006BC loc_4006BC:                             ; CODE XREF: fibonacci+C2j
.text:0x4006BC                 and     edx, 33333333h
.text:0x4006C2                 lea     ecx, [rsi+rdx]
.text:0x4006C5                 mov     edx, ecx
.text:0x4006C7                 shr     edx, 4
.text:0x4006CA                 add     edx, ecx
.text:0x4006CC                 mov     esi, edx
.text:0x4006CE                 and     edx, 0F0F0F0Fh
.text:0x4006D4                 shr     esi, 8
.text:0x4006D7                 and     esi, 0F0F0Fh
.text:0x4006DD                 lea     ecx, [rsi+rdx]
.text:0x4006E0                 mov     edx, ecx
.text:0x4006E2                 shr     edx, 10h
.text:0x4006E5                 add     edx, ecx
.text:0x4006E7                 and     edx, 1
.text:0x4006EA                 xor     [rbp+0], edx
.text:0x4006ED                 pop     rbx
.text:0x4006EE                 pop     rbp
.text:0x4006EF                 pop     r12
.text:0x4006F1                 retn
.text:0x4006F1 ; ---------------------------------------------------------------------------
.text:0x4006F2                 align 8
.text:0x4006F8
.text:0x4006F8 loc_4006F8:                             ; CODE XREF: fibonacci+9j
.text:0x4006F8                 mov     edx, 1
.text:0x4006FD                 xor     [rbp+0], edx
.text:0x400700                 mov     eax, 1
.text:0x400705                 pop     rbx
.text:0x400706                 pop     rbp
.text:0x400707                 pop     r12
.text:0x400709                 retn
.text:0x400709 ; ---------------------------------------------------------------------------
.text:0x40070A                 align 10h
.text:0x400710
.text:0x400710 loc_400710:                             ; CODE XREF: fibonacci+10j
.text:0x400710                 xor     edi, edi
.text:0x400712                 call    fibonacci
.text:0x400717                 mov     edx, eax
.text:0x400719                 mov     edi, eax
.text:0x40071B                 shr     edx, 1
.text:0x40071D                 and     edx, 55555555h
.text:0x400723                 sub     edi, edx
.text:0x400725                 mov     esi, edi
.text:0x400727                 mov     edx, edi
.text:0x400729                 shr     esi, 2
.text:0x40072C                 and     esi, 33333333h
.text:0x400732                 jmp     short loc_4006BC
.text:0x400732 fibonacci       endp
```

解决这个问题的方式有很多种。例如，我们可以使用一种编程语言重新构建代码，并对新构建的代码进行优化。重建代码的过程并不容易，并且有可能会产生问题或错误，而解决问题、修正错误的这个过程是非常煎熬的。但假如我们使用Unicorn Engine，就可以跳过重建代码的过程，从而避免上面提到的问题。我们还可以通过其他几种方法跳过重建代码的过程，例如通过脚本调试，或者是使用Frida。
在优化之前，我们首先模拟正常的程序，一旦程序成功运行后，我们再在Unicorn Engine中对其进行优化。

### 第一部分：模拟程序

首先我们创建一个名为fibonacci.py的文件，并将二进制文件放在同一个文件夹下。
将下面的代码添加到文件中：

```
from unicorn import *
from unicorn.x86_const import *
```

其中，第一行加载主二进制程序以及基本的Unicorn Constant，第二行加载特定于x86和x86-64体系结构的Constant。
接下来，添加如下几行：

```
import struct

def read(name):
    with open(name) as f:
        return f.read()

def u32(data):
    return struct.unpack("I", data)[0]

def p32(num):
    return struct.pack("I", num)
```

在这里，我们只添加了一些通常的功能，这些功能稍后会对我们有所帮助。
其中，read会返回整个文件的内容。u32需要一个4字节的字符串，并将其转换为一个整数，以低字节序表示这个数据。p32正相反，它需要一个数字，并将其转换为4字节的字符串，以低字节序表示。
如果你安装了pwntools，那么你就不需要创建这些函数，只需要通过`pwn import *`导入即可。
接下来，让我们初始化我们Unicorn Engine的类，以适应x86-64架构：

```
mu = Uc (UC_ARCH_X86, UC_MODE_64)
```

我们需要使用下面的参数来调用函数Uc：
1、主结构分支，其中的Constant以UC*ARCH*开始；
2、进一步的架构规范，其中的Constant以UC*MODE*开始。
您可以在本文后面的参考内容中，找到架构Constant的完整列表。
正如我们之前所说的，要使用Unicorn Engine，我们需要手动初始化虚拟内存。对于这个二进制文件，我们需要在其中的某个位置编写代码，并分配一个栈。
二进制的基址是0x400000。我们的栈将从地址0x000000开始，大小为1024*1024。也许我们并不需要那么大的空间，但创建大一些的空间也不会有任何不好的影响。
我们可以通过调用mem_map方法来映射内存。
添加如下行：

```
BASE = 0x400000
STACK_ADDR = 0x0
STACK_SIZE = 1024*1024

mu.mem_map(BASE, 1024*1024)
mu.mem_map(STACK_ADDR, STACK_SIZE)
```

现在，我们需要在基址加载二进制文件，就像加载器一样。然后我们需要将RSP设置为指向栈的末尾。

```
mu.mem_write(BASE, read("./fibonacci"))
mu.reg_write(UC_X86_REG_RSP, STACK_ADDR + STACK_SIZE - 1)
```

在开始模拟并运行代码之前，我们首先需要知道开始地址在哪里，并且要知道模拟器应该在哪里停止。
我们可以开始模拟位于地址0x4004E0的代码，这是main的第一个地址。结束位置可以选择0x400575，这是`putc("n")`的位置，会在打印完整个Flag后被调用。如下所示：

```
.text:0x400570                 mov     edi, 0Ah        ; c
.text:0x400575                 call    __IO_putc
```

我们可以开始模拟：

```
mu.emu_start(0x00000000004004E0, 0x0000000000400575)
```

现在，可以运行这个脚本：

```
a@x:~/Desktop/unicorn_engine_lessons$ python solve.py 
Traceback (most recent call last):
  File "solve.py", line 32, in <module>
    mu.emu_start(0x00000000004004E0, 0x0000000000400575)
  File "/usr/local/lib/python2.7/dist-packages/unicorn/unicorn.py", line 288, in emu_start
    raise UcError(status)
unicorn.unicorn.UcError: Invalid memory read (UC_ERR_READ_UNMAPPED)
```

在这时，我们发现出现了一些问题，但具体还不得而知。在mu.emu_start之前，我们可以添加：

```
def hook_code(mu, address, size, user_data):  
    print('>>> Tracing instruction at 0x%x, instruction size = 0x%x' %(address, size)) 

mu.hook_add(UC_HOOK_CODE, hook_code)
```

这段代码添加了一个钩子。我们定义了函数hook_code，在模拟每个指令前被调用。该函数需要以下参数：
1、Uc实例
2、指令的地址
3、指令的大小
4、用户数据（我们可以在hook_add()的可选参数中传递这个值）
相关源代码请参考solve1.py：http://eternal.red/assets/files/2017/UE/solve1.py 。
运行时，我们可以看到：

```
a@x:~/Desktop/unicorn_engine_lessons$ python solve.py 
>>> Tracing instruction at 0x4004e0, instruction size = 0x1
>>> Tracing instruction at 0x4004e1, instruction size = 0x1
>>> Tracing instruction at 0x4004e2, instruction size = 0x2
>>> Tracing instruction at 0x4004e4, instruction size = 0x5
>>> Tracing instruction at 0x4004e9, instruction size = 0x2
>>> Tracing instruction at 0x4004eb, instruction size = 0x4
>>> Tracing instruction at 0x4004ef, instruction size = 0x7
Traceback (most recent call last):
  File "solve.py", line 41, in <module>
    mu.emu_start(0x00000000004004E0, 0x0000000000400575)
  File "/usr/local/lib/python2.7/dist-packages/unicorn/unicorn.py", line 288, in emu_start
    raise UcError(status)
unicorn.unicorn.UcError: Invalid memory read (UC_ERR_READ_UNMAPPED)
```

这意味着，脚本在执行以下指令时失败：

```
.text:0x4004EF                 mov     rdi, cs:stdout  ; stream
```

该指令从地址0x601038读取内存（可以在IDA Pro中看到）。这是.bss段，并不是由我们分配的。因此我们的解决方案是跳过所有有问题的指令。
下面有一条指令：

```
.text:0x4004F6                 call    _setbuf
```

我们并不能调用任何glibc函数，因为此前并没有将glibc加载到虚拟内存中。事实上，我们并不需要调用这个函数，所以也可以跳过它。
下面是我们需要跳过的指令列表：

```
.text:0x4004EF                 mov     rdi, cs:stdout  ; stream
.text:0x4004F6                 call    _setbuf
.text:0x400502                 call    _printf
.text:0x40054F                 mov     rsi, cs:stdout  ; fp
```

我们可以通过将地址写入下一条指令的RIP寄存器来跳过指令：

```
mu.reg_write(UC_X86_REG_RIP, address+size)
```

hook_code现在应该是这样的：

```
instructions_skip_list = [0x00000000004004EF, 0x00000000004004F6, 0x0000000000400502, 0x000000000040054F]

def hook_code(mu, address, size, user_data):  
    print('>>> Tracing instruction at 0x%x, instruction size = 0x%x' %(address, size))

    if address in instructions_skip_list:
        mu.reg_write(UC_X86_REG_RIP, address+size)
```

此外，我们还需要对逐字节打印Flag的指令进行一些操作。

```
.text:0x400558                 movsx   edi, dil        ; c
.text:0x40055C                 add     rbp, 1
.text:0x400560                 call    __IO_putc
```

__IO_putc需要一个字节，以打印出第一个参数（即寄存器RDI）。
我们可以从寄存器RDI中读取一个值并打印出来，同时跳过模拟这个指令。此时的hook_code函数如下所示：

```
instructions_skip_list = [0x00000000004004EF, 0x00000000004004F6, 0x0000000000400502, 0x000000000040054F]

def hook_code(mu, address, size, user_data):  
    #print('>>> Tracing instruction at 0x%x, instruction size = 0x%x' %(address, size))

    if address in instructions_skip_list:
        mu.reg_write(UC_X86_REG_RIP, address+size)

    elif address == 0x400560: #that instruction writes a byte of the flag
        c = mu.reg_read(UC_X86_REG_RDI)
        print(chr(c))
        mu.reg_write(UC_X86_REG_RIP, address+size)
```

相关源代码请参考solve2.py：http://eternal.red/assets/files/2017/UE/solve2.py 。
接下来，便可以运行，我们发现它确实可以正常工作，但速度还是很慢。

```
a@x:~/Desktop/unicorn_engine_lessons$ python solve.py 
h
x
```

### 第二部分：提速

接下来，让我们考虑一下提速的方法。为什么这个程序的运行速度如此之慢？
查看反编译的代码，我们可以看到main()多次调用了fibonacci()，并且fibonacci()是一个递归函数。
具体分析这个函数，我们看到它有两个参数，并返回两个值。第一个返回值通过RAX寄存器传递，而第二个返回值通过第二个参数传递。深入研究main()和fibonacci()，我们注意到其第二个参数只能取0或1的值。如果我们没有发现，还可以运行gdb，并在fibonacci()函数的开始处设置一个断点。
为了优化这个函数，我们可以使用动态编程的方法来记录针对特定参数的返回值。由于第二个参数只可能是两个值，所以我们只需要记录2个MAX_OF_FIRST_ARGUMENT对。
当RIP指向fibonacci函数的开始时，我们可以获得函数的参数。在函数结束时，需要得知函数的返回值。既然目前我们不清楚返回值，所以需要使用一个栈，来帮助我们在函数结束时获得这两个返回值。在fibonacci的入口，我们需要将参数推入栈，并在最后弹出。为了记录其中的对（Pairs），我们可以使用字典。
如何检查对（Pairs）的值？
在函数的开始处，可以检查返回值是否被存储在字典中，以用于这些参数。如果已经被存储，我们可以返回该对。只需要将返回值写入到引用和RAX中即可。此外，我们还将RIP设置为一些RET指令的地址来退出函数。由于这一指令被Hook住了，所以我们不能在fibonacci函数中跳转到RET。如果该返回值不在字典中，我们将参数添加到栈中。在退出函数时，可以保存返回值。我们可以从栈结构中读取参数和引用指针。
代码如下所示：

```
FIBONACCI_ENTRY = 0x0000000000400670
FIBONACCI_END = [0x00000000004006F1, 0x0000000000400709]

stack = []                                          # Stack for storing the arguments
d = {}                                              # Dictionary that holds return values for given function arguments 

def hook_code(mu, address, size, user_data):  
    #print('>>> Tracing instruction at 0x%x, instruction size = 0x%x' %(address, size))

    if address in instructions_skip_list:
        mu.reg_write(UC_X86_REG_RIP, address+size)

    elif address == 0x400560:                       # That instruction writes a byte of the flag
        c = mu.reg_read(UC_X86_REG_RDI)
        print(chr(c))
        mu.reg_write(UC_X86_REG_RIP, address+size)

    elif address == FIBONACCI_ENTRY:                # Are we at the beginning of fibonacci function?
        arg0 = mu.reg_read(UC_X86_REG_RDI)          # Read the first argument. Tt is passed via RDI
        r_rsi = mu.reg_read(UC_X86_REG_RSI)         # Read the second argument which is a reference
        arg1 = u32(mu.mem_read(r_rsi, 4))           # Read the second argument from reference

        if (arg0,arg1) in d:                        # Check whether return values for this function are already saved.
            (ret_rax, ret_ref) = d[(arg0,arg1)]
            mu.reg_write(UC_X86_REG_RAX, ret_rax)   # Set return value in RAX register
            mu.mem_write(r_rsi, p32(ret_ref))       # Set retun value through reference
            mu.reg_write(UC_X86_REG_RIP, 0x400582)  # Set RIP to point at RET instruction. We want to return from fibonacci function

        else:
            stack.append((arg0,arg1,r_rsi))         # If return values are not saved for these arguments, add them to stack.

    elif address in FIBONACCI_END:
        (arg0, arg1, r_rsi) = stack.pop()           # We know arguments when exiting the function

        ret_rax = mu.reg_read(UC_X86_REG_RAX)       # Read the return value that is stored in RAX
        ret_ref = u32(mu.mem_read(r_rsi,4))         # Read the return value that is passed reference
        d[(arg0, arg1)]=(ret_rax, ret_ref)          # Remember the return values for this argument pair
```

完整脚本请参考solve3.py：http://eternal.red/assets/files/2017/UE/solve3.py 。

```
from unicorn import *
from unicorn.x86_const import *


import struct

def read(name):
    with open(name) as f:
        return f.read()
        
def u32(data):
    return struct.unpack("I", data)[0]
    
def p32(num):
    return struct.pack("I", num)


mu = Uc (UC_ARCH_X86, UC_MODE_64)


BASE = 0x400000
STACK_ADDR = 0x0
STACK_SIZE = 1024*1024

mu.mem_map(BASE, 1024*1024)
mu.mem_map(STACK_ADDR, STACK_SIZE)


mu.mem_write(BASE, read("./fibonacci"))
mu.reg_write(UC_X86_REG_RSP, STACK_ADDR + STACK_SIZE - 1)

instructions_skip_list = [0x00000000004004EF, 0x00000000004004F6, 0x0000000000400502, 0x000000000040054F]

FIBONACCI_ENTRY = 0x0000000000400670
FIBONACCI_END = [0x00000000004006F1, 0x0000000000400709]

stack = []                                          # Stack for storing the arguments
d = {}                                              # Dictionary that holds return values for given function arguments 

def hook_code(mu, address, size, user_data):  
    #print('>>> Tracing instruction at 0x%x, instruction size = 0x%x' %(address, size))
    
    if address in instructions_skip_list:
        mu.reg_write(UC_X86_REG_RIP, address+size)
    
    elif address == 0x400560:                       # That instruction writes a byte of the flag
        c = mu.reg_read(UC_X86_REG_RDI)
        print(chr(c))
        mu.reg_write(UC_X86_REG_RIP, address+size)
    
    elif address == FIBONACCI_ENTRY:                # Are we at the beginning of fibonacci function?
        arg0 = mu.reg_read(UC_X86_REG_RDI)          # Read the first argument. Tt is passed via RDI
        r_rsi = mu.reg_read(UC_X86_REG_RSI)         # Read the second argument which is a reference
        arg1 = u32(mu.mem_read(r_rsi, 4))           # Read the second argument from reference
        
        if (arg0,arg1) in d:                        # Check whether return values for this function are already saved.
            (ret_rax, ret_ref) = d[(arg0,arg1)]
            mu.reg_write(UC_X86_REG_RAX, ret_rax)   # Set return value in RAX register
            mu.mem_write(r_rsi, p32(ret_ref))       # Set retun value through reference
            mu.reg_write(UC_X86_REG_RIP, 0x400582)  # Set RIP to point at RET instruction. We want to return from fibonacci function
            
        else:
            stack.append((arg0,arg1,r_rsi))         # If return values are not saved for these arguments, add them to stack.
        
    elif address in FIBONACCI_END:
        (arg0, arg1, r_rsi) = stack.pop()           # We know arguments when exiting the function
        
        ret_rax = mu.reg_read(UC_X86_REG_RAX)       # Read the return value that is stored in RAX
        ret_ref = u32(mu.mem_read(r_rsi,4))         # Read the return value that is passed reference
        d[(arg0, arg1)]=(ret_rax, ret_ref)          # Remember the return values for this argument pair


mu.hook_add(UC_HOOK_CODE, hook_code)


mu.emu_start(0x00000000004004E0, 0x0000000000400575)

```





至此，我们已经成功地使用Unicorn Engine来优化程序。
接下来，我推荐大家完成下面三个任务的练习。针对每个任务，都有提示和解决方案，并且在解决任务的过程中，可以查看后文的参考内容。
我认为，其中一个很重要的问题就是要知道Constant的名称。处理这个问题的最好方法是借助IPython的自动补全（Tab Completion）来完成。在你安装IPython之后，可以输入from unicorn import UC*ARCH*，并按TAB键，所有以这个前缀开头的Constant都将被打印出来。

## 

## 任务2

分析下列Shellcode：

```
shellcode = "xe8xffxffxffxffxc0x5dx6ax05x5bx29xddx83xc5x4ex89xe9x6ax02x03x0cx24x5bx31xd2x66xbax12x00x8bx39xc1xe7x10xc1xefx10x81xe9xfexffxffxffx8bx45x00xc1xe0x10xc1xe8x10x89xc3x09xfbx21xf8xf7xd0x21xd8x66x89x45x00x83xc5x02x4ax85xd2x0fx85xcfxffxffxffxecx37x75x5dx7ax05x28xedx24xedx24xedx0bx88x7fxebx50x98x38xf9x5cx96x2bx96x70xfexc6xffxc6xffx9fx32x1fx58x1ex00xd3x80"
```

如你所见，该程序集被混淆了（命令disarm是pwntools的一个功能）：

```
a@x:~/Desktop/unicorn_engine_lessons$ disasm e8ffffffffc05d6a055b29dd83c54e89e96a02030c245b31d266ba12008b39c1e710c1ef1081e9feffffff8b4500c1e010c1e81089c309fb21f8f7d021d86689450083c5024a85d20f85cfffffffec37755d7a0528ed24ed24ed0b887feb509838f95c962b9670fec6ffc6ff9f321f581e00d380
   0:    e8 ff ff ff ff           call   0x4
   5:    c0 5d 6a 05              rcr    BYTE PTR [ebp+0x6a], 0x5
   9:    5b                       pop    ebx
   a:    29 dd                    sub    ebp, ebx
   c:    83 c5 4e                 add    ebp, 0x4e
   f:    89 e9                    mov    ecx, ebp
  11:    6a 02                    push   0x2
  13:    03 0c 24                 add    ecx, DWORD PTR [esp]
  16:    5b                       pop    ebx
  17:    31 d2                    xor    edx, edx
  19:    66 ba 12 00              mov    dx, 0x12
  1d:    8b 39                    mov    edi, DWORD PTR [ecx]
  1f:    c1 e7 10                 shl    edi, 0x10
  22:    c1 ef 10                 shr    edi, 0x10
  25:    81 e9 fe ff ff ff        sub    ecx, 0xfffffffe
  2b:    8b 45 00                 mov    eax, DWORD PTR [ebp+0x0]
  2e:    c1 e0 10                 shl    eax, 0x10
  31:    c1 e8 10                 shr    eax, 0x10
  34:    89 c3                    mov    ebx, eax
  36:    09 fb                    or     ebx, edi
  38:    21 f8                    and    eax, edi
  3a:    f7 d0                    not    eax
  3c:    21 d8                    and    eax, ebx
  3e:    66 89 45 00              mov    WORD PTR [ebp+0x0], ax
  42:    83 c5 02                 add    ebp, 0x2
  45:    4a                       dec    edx
  46:    85 d2                    test   edx, edx
  48:    0f 85 cf ff ff ff        jne    0x1d
  4e:    ec                       in     al, dx
  4f:    37                       aaa
  50:    75 5d                    jne    0xaf
  52:    7a 05                    jp     0x59
  54:    28 ed                    sub    ch, ch
  56:    24 ed                    and    al, 0xed
  58:    24 ed                    and    al, 0xed
  5a:    0b 88 7f eb 50 98        or     ecx, DWORD PTR [eax-0x67af1481]
  60:    38 f9                    cmp    cl, bh
  62:    5c                       pop    esp
  63:    96                       xchg   esi, eax
  64:    2b 96 70 fe c6 ff        sub    edx, DWORD PTR [esi-0x390190]
  6a:    c6                       (bad)
  6b:    ff 9f 32 1f 58 1e        call   FWORD PTR [edi+0x1e581f32]
  71:    00 d3                    add    bl, dl
  73:    80                       .byte 0x80
```

请注意，目前的架构是x86-32。系统调用的列表可以在这里查看：https://syscalls.kernelgrok.com/ 。

### 提示

您可以Hook一个int 80h指令，它由cd 80表示。接下来，您可以读取寄存器和内存。需要记住的是，Shellcode是一个可以在任何地址加载的代码，绝大多数Shellcode都使用了栈。

### 解决方案

下面的代码是通过几个步骤创建而成的。通过UE错误信息，我们获得了一些线索，并想到了最终的解决方案。

```
from unicorn import *
from unicorn.x86_const import *

shellcode = "xe8xffxffxffxffxc0x5dx6ax05x5bx29xddx83xc5x4ex89xe9x6ax02x03x0cx24x5bx31xd2x66xbax12x00x8bx39xc1xe7x10xc1xefx10x81xe9xfexffxffxffx8bx45x00xc1xe0x10xc1xe8x10x89xc3x09xfbx21xf8xf7xd0x21xd8x66x89x45x00x83xc5x02x4ax85xd2x0fx85xcfxffxffxffxecx37x75x5dx7ax05x28xedx24xedx24xedx0bx88x7fxebx50x98x38xf9x5cx96x2bx96x70xfexc6xffxc6xffx9fx32x1fx58x1ex00xd3x80" 


BASE = 0x400000
STACK_ADDR = 0x0
STACK_SIZE = 1024*1024

mu = Uc (UC_ARCH_X86, UC_MODE_32)

mu.mem_map(BASE, 1024*1024)
mu.mem_map(STACK_ADDR, STACK_SIZE)


mu.mem_write(BASE, shellcode)
mu.reg_write(UC_X86_REG_ESP, STACK_ADDR + STACK_SIZE/2)

def syscall_num_to_name(num):
    syscalls = {1: "sys_exit", 15: "sys_chmod"}
    return syscalls[num]

def hook_code(mu, address, size, user_data):
    #print('>>> Tracing instruction at 0x%x, instruction size = 0x%x' %(address, size))  

    machine_code = mu.mem_read(address, size)
    if machine_code == "xcdx80":

        r_eax = mu.reg_read(UC_X86_REG_EAX)
        r_ebx = mu.reg_read(UC_X86_REG_EBX)
        r_ecx = mu.reg_read(UC_X86_REG_ECX)
        r_edx = mu.reg_read(UC_X86_REG_EDX)
        syscall_name = syscall_num_to_name(r_eax)

        print "--------------"
        print "We intercepted system call: "+syscall_name

        if syscall_name == "sys_chmod":
            s = mu.mem_read(r_ebx, 20).split("x00")[0]
            print "arg0 = 0x%x -> %s" % (r_ebx, s)
            print "arg1 = " + oct(r_ecx)
        elif syscall_name == "sys_exit":
            print "arg0 = " + hex(r_ebx)
            exit()

        mu.reg_write(UC_X86_REG_EIP, address + size)

mu.hook_add(UC_HOOK_CODE, hook_code)

mu.emu_start(BASE, BASE-1)
```

最终代码如下：

```
a@x:~/Desktop/unicorn_engine_lessons$ python solve_task2.py
--------------
We intercepted system call: sys_chmod
arg0 = 0x400058 -> /etc/shadow
arg1 = 0666L
--------------
We intercepted system call: sys_exit
arg0 = 0x400058L
```

## 

## 任务3

下载二进制文件（ http://eternal.red/assets/files/2017/UE/function ），该文件是用以下命令编译的：
`gcc function.c -m32 -o function`
这个二进制代码如下所示：

```
int strcmp(char *a, char *b)
{
    //get length
    int len = 0;
    char *ptr = a;
    while(*ptr)
    {
        ptr++;
        len++;
    }

    //comparestrings
    for(int i=0; i<=len; i++)
    {
        if (a[i]!=b[i])
            return 1;
    }

    return 0;
}

__attribute__((stdcall))
int  super_function(int a, char *b)
{
    if (a==5 && !strcmp(b, "batman"))
    {
        return 1;
    }
    return 0;
}

int main()
{
    super_function(1, "spiderman");
}
```

任务是调用super_function，使其返回1。
其汇编代码如下：

```
.text:0x8048464 super_function  proc near               ; CODE XREF: main+16p
.text:0x8048464
.text:0x8048464 arg_0           = dword ptr  8
.text:0x8048464 arg_4           = dword ptr  0Ch
.text:0x8048464
.text:0x8048464                 push    ebp
.text:0x8048465                 mov     ebp, esp
.text:0x8048467                 call    __x86_get_pc_thunk_ax
.text:0x804846C                 add     eax, 1B94h
.text:0x8048471                 cmp     [ebp+arg_0], 5
.text:0x8048475                 jnz     short loc_8048494
.text:0x8048477                 lea     eax, (aBatman - 804A000h)[eax] ; "batman"
.text:0x804847D                 push    eax
.text:0x804847E                 push    [ebp+arg_4]
.text:0x8048481                 call    strcmp
.text:0x8048486                 add     esp, 8
.text:0x8048489                 test    eax, eax
.text:0x804848B                 jnz     short loc_8048494
.text:0x804848D                 mov     eax, 1
.text:0x8048492                 jmp     short locret_8048499
.text:0x8048494 ; ---------------------------------------------------------------------------
.text:0x8048494
.text:0x8048494 loc_8048494:                            ; CODE XREF: super_function+11j
.text:0x8048494                                         ; super_function+27j
.text:0x8048494                 mov     eax, 0
.text:0x8048499
.text:0x8048499 locret_8048499:                         ; CODE XREF: super_function+2Ej
.text:0x8048499                 leave
.text:0x804849A                 retn    8
.text:0x804849A super_function  endp
```

### 提示

根据stdcall调用约定，当模拟过程开始时，栈应该如下图所示。我们看到在下图中，RET只是返回地址（可以为任意值）。
[![img](data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAAAXNSR0IArs4c6QAAAARnQU1BAACxjwv8YQUAAAAJcEhZcwAADsQAAA7EAZUrDhsAAAANSURBVBhXYzh8+PB/AAffA0nNPuCLAAAAAElFTkSuQmCC)](https://eternal.red/assets/files/2017/UE/stack.png)

### 解决方案

```
from unicorn import *
from unicorn.x86_const import *
import struct


def read(name):
    with open(name) as f:
        return f.read()

def u32(data):
    return struct.unpack("I", data)[0]

def p32(num):
    return struct.pack("I", num)

mu = Uc (UC_ARCH_X86, UC_MODE_32)

BASE = 0x08048000
STACK_ADDR = 0x0
STACK_SIZE = 1024*1024

mu.mem_map(BASE, 1024*1024)
mu.mem_map(STACK_ADDR, STACK_SIZE)


mu.mem_write(BASE, read("./function"))
r_esp = STACK_ADDR + (STACK_SIZE/2)     #ESP points to this address at function call

STRING_ADDR = 0x0
mu.mem_write(STRING_ADDR, "batmanx00") #write "batman" somewhere. We have choosen an address 0x0 which belongs to the stack.

mu.reg_write(UC_X86_REG_ESP, r_esp)     #set ESP
mu.mem_write(r_esp+4, p32(5))           #set the first argument. It is integer 5
mu.mem_write(r_esp+8, p32(STRING_ADDR)) #set the second argument. This is a pointer to the string "batman"


mu.emu_start(0x8048464, 0x804849A)      #start emulation from the beginning of super_function, end at RET instruction
return_value = mu.reg_read(UC_X86_REG_EAX)
print "The returned value is: %d" % return_value
a@x:~/Desktop/unicorn_engine_lessons$ python solve_task3.py 
The returned value is: 1
```

## 

## 任务4

这个任务与任务1类似，但不同之处就在于这里的架构不再是x86，而是低字节序的ARM32。

```
a@x:~/Desktop/unicorn_engine_lessons$ file task4
task4: ELF 32-bit LSB executable, ARM, EABI5 version 1 (SYSV), statically linked, for GNU/Linux 3.2.0, BuildID[sha1]=3dbf508680ba3d023d3422025954311e1d8fb4a1, not stripped
```

二进制文件下载地址为：http://eternal.red/assets/files/2017/UE/task4 。
参考这篇资料可能会有所帮助：http://infocenter.arm.com/help/topic/com.arm.doc.ihi0042f/IHI0042F_aapcs.pdf 。

### 正确答案

2635833876

### 提示

1、函数的第一个参数在R0 (UC_ARM_REG_R0)中传递；
2、返回值也在R0中；
3、第二个参数在R1 (UC_ARM_REG_R1)中传递；
4、我们可以通过这种方式来得到ARM32架构下的Unicorn实例：mu = Uc (UC_ARCH_ARM, UC_MODE_LITTLE_ENDIAN)。

### 解决方案

```
from unicorn import *
from unicorn.arm_const import *


import struct

def read(name):
    with open(name) as f:
        return f.read()

def u32(data):
    return struct.unpack("I", data)[0]

def p32(num):
    return struct.pack("I", num)


mu = Uc (UC_ARCH_ARM, UC_MODE_LITTLE_ENDIAN)


BASE = 0x10000
STACK_ADDR = 0x300000
STACK_SIZE = 1024*1024

mu.mem_map(BASE, 1024*1024)
mu.mem_map(STACK_ADDR, STACK_SIZE)


mu.mem_write(BASE, read("./task4"))
mu.reg_write(UC_ARM_REG_SP, STACK_ADDR + STACK_SIZE/2)

instructions_skip_list = []

CCC_ENTRY = 0x000104D0
CCC_END = 0x00010580

stack = []                                          # Stack for storing the arguments
d = {}                                              # Dictionary that holds return values for given function arguments 

def hook_code(mu, address, size, user_data):  
    #print('>>> Tracing instruction at 0x%x, instruction size = 0x%x' %(address, size))

    if address == CCC_ENTRY:                        # Are we at the beginning of ccc function?
        arg0 = mu.reg_read(UC_ARM_REG_R0)           # Read the first argument. it is passed by R0

        if arg0 in d:                               # Check whether return value for this function is already saved.
            ret = d[arg0]
            mu.reg_write(UC_ARM_REG_R0, ret)        # Set return value in R0
            mu.reg_write(UC_ARM_REG_PC, 0x105BC)    # Set PC to point at "BX LR" instruction. We want to return from fibonacci function

        else:
            stack.append(arg0)                      # If return value is not saved for this argument, add it to stack.

    elif address == CCC_END:
        arg0 = stack.pop()                          # We know arguments when exiting the function

        ret = mu.reg_read(UC_ARM_REG_R0)            # Read the return value (R0)
        d[arg0] = ret                               # Remember the return value for this argument

mu.hook_add(UC_HOOK_CODE, hook_code)

mu.emu_start(0x00010584, 0x000105A8)

return_value = mu.reg_read(UC_ARM_REG_R1)           # We end the emulation at printf("%dn", ccc(x)).
print "The return value is %d" % return_value
```

## 参考内容

`from unicorn import *` —— 加载主Unicorn库，它包含函数和基本Constant。
`from unicorn.x86_const import *` —— 加载特定于x86和x86-64架构的Constant。
Unicorn模块中的所有Const如下：

```
UC_API_MAJOR                UC_ERR_VERSION              UC_MEM_READ                 UC_PROT_ALL
UC_API_MINOR                UC_ERR_WRITE_PROT           UC_MEM_READ_AFTER           UC_PROT_EXEC
UC_ARCH_ARM                 UC_ERR_WRITE_UNALIGNED      UC_MEM_READ_PROT            UC_PROT_NONE
UC_ARCH_ARM64               UC_ERR_WRITE_UNMAPPED       UC_MEM_READ_UNMAPPED        UC_PROT_READ
UC_ARCH_M68K                UC_HOOK_BLOCK               UC_MEM_WRITE                UC_PROT_WRITE
UC_ARCH_MAX                 UC_HOOK_CODE                UC_MEM_WRITE_PROT           UC_QUERY_MODE
UC_ARCH_MIPS                UC_HOOK_INSN                UC_MEM_WRITE_UNMAPPED       UC_QUERY_PAGE_SIZE
UC_ARCH_PPC                 UC_HOOK_INTR                UC_MILISECOND_SCALE         UC_SECOND_SCALE
UC_ARCH_SPARC               UC_HOOK_MEM_FETCH           UC_MODE_16                  UC_VERSION_EXTRA
UC_ARCH_X86                 UC_HOOK_MEM_FETCH_INVALID   UC_MODE_32                  UC_VERSION_MAJOR
UC_ERR_ARCH                 UC_HOOK_MEM_FETCH_PROT      UC_MODE_64                  UC_VERSION_MINOR
UC_ERR_ARG                  UC_HOOK_MEM_FETCH_UNMAPPED  UC_MODE_ARM                 Uc
UC_ERR_EXCEPTION            UC_HOOK_MEM_INVALID         UC_MODE_BIG_ENDIAN          UcError
UC_ERR_FETCH_PROT           UC_HOOK_MEM_PROT            UC_MODE_LITTLE_ENDIAN       arm64_const
UC_ERR_FETCH_UNALIGNED      UC_HOOK_MEM_READ            UC_MODE_MCLASS              arm_const
UC_ERR_FETCH_UNMAPPED       UC_HOOK_MEM_READ_AFTER      UC_MODE_MICRO               debug
UC_ERR_HANDLE               UC_HOOK_MEM_READ_INVALID    UC_MODE_MIPS3               m68k_const
UC_ERR_HOOK                 UC_HOOK_MEM_READ_PROT       UC_MODE_MIPS32              mips_const
UC_ERR_HOOK_EXIST           UC_HOOK_MEM_READ_UNMAPPED   UC_MODE_MIPS32R6            sparc_const
UC_ERR_INSN_INVALID         UC_HOOK_MEM_UNMAPPED        UC_MODE_MIPS64              uc_arch_supported
UC_ERR_MAP                  UC_HOOK_MEM_VALID           UC_MODE_PPC32               uc_version
UC_ERR_MODE                 UC_HOOK_MEM_WRITE           UC_MODE_PPC64               unicorn
UC_ERR_NOMEM                UC_HOOK_MEM_WRITE_INVALID   UC_MODE_QPX                 unicorn_const
UC_ERR_OK                   UC_HOOK_MEM_WRITE_PROT      UC_MODE_SPARC32             version_bind
UC_ERR_READ_PROT            UC_HOOK_MEM_WRITE_UNMAPPED  UC_MODE_SPARC64             x86_const
UC_ERR_READ_UNALIGNED       UC_MEM_FETCH                UC_MODE_THUMB               
UC_ERR_READ_UNMAPPED        UC_MEM_FETCH_PROT           UC_MODE_V8                  
UC_ERR_RESOURCE             UC_MEM_FETCH_UNMAPPED       UC_MODE_V9
```

来自unicorn.x86_const的一些Constant示例：
`UC_X86_REG_EAX`
`UC_X86_REG_RIP`
`UC_X86_REG_RAX`
`mu = Uc(arch, mode)` —— 获得一个Uc类的实例，在这里可以指定架构。
举例来说：
`mu = Uc(UC_ARCH_X86, UC_MODE_64)` 获得一个x86-64架构的Uc实例。
`mu = Uc(UC_ARCH_X86, UC_MODE_32)` 获得一个x86-32架构的Uc实例。
`mu.mem_map(ADDRESS, 4096)` 映射一个内存区域。
`mu.mem_write(ADDRESS, DATA)` 将数据写入内存。
`tmp = mu.mem_read(ADDRESS, SIZE)` 从内存中读取数据。
`mu.reg_write(UC_X86_REG_ECX, 0x0)` 将寄存器重新赋值。
`r_esp = mu.reg_read(UC_X86_REG_ESP)` 读取寄存器的值。
`mu.emu_start(ADDRESS_START, ADDRESS_END)` 开始模拟。
指令跟踪：

```
def hook_code(mu, address, size, user_data):  
    print('>>> Tracing instruction at 0x%x, instruction size = 0x%x' %(address, size))  

mu.hook_add(UC_HOOK_CODE, hook_code)
```

这段代码添加了一个钩子。我们定义了函数hook_code，在模拟每个指令之前调用，该函数需要以下参数：
1、Uc实例
2、指令的地址
3、指令的大小
4、用户数据（我们可以在hook_add()的可选参数中传递这个值）

## 

## 参考资料

1. 关于Unicorn Engine的基本介绍：
   http://www.unicorn-engine.org/BHUSA2015-unicorn.pdf
2. Oh look, there are bindings for many languages：
   https://github.com/unicorn-engine/unicorn/tree/master/bindings
3. Unicorn Engine参考：
   https://hackmd.io/s/rJTUtGwuW#
4. 官方UE教程：
   http://www.unicorn-engine.org/docs/tutorial.html