# VMP虚拟机加壳的原理学习

https://www.cnblogs.com/LittleHann/p/3344261.html

```
好久没有到博客写文章了，9月份开学有点忙，参加了一个上海的一个CHINA SIG信息比赛，前几天又无锡南京来回跑了几趟，签了阿里巴巴的安全工程师，准备11月以后过去实习，这之前就好好待在学校学习了。

这段时间断断续续把《加密与解码 第三版》给看完了，虽然对逆向还是一知半解，不过对VMP虚拟机加壳这块有了一点新的认识。这里分享一些笔记和想法，没有新的东西，都是书上还KSSD里看来的，权当笔记和分享，大神勿喷。

准备分成3部分讲一下

1. VM虚拟机简介

2. VM虚拟指令和x86汇编的映射原理

3. 用VM虚拟指令构造一段小程序

 

1. VM虚拟机简介

虚拟机保护技术就是将基于x86汇编系统的可执行代码转换为字节码指令系统的代码，以达到保护原有指令不被轻易逆向和修改的目的，这种指令也可以叫伪指令，和VB的pcode有点类似。

从本质上讲，虚拟指令系统就是对原本的x86汇编指令系统进行一次封装，将原本的汇编指令转换为另一种表现形式。

例如：

push uType
push lpCaption
push lpText
push hWnd,
call MessageBox

这是一段x86的汇编指令，编译器在翻译的时候会产生一个固定模式的汇编代码(在同一个CPU指令集下)。
但如果我们对原本的C代码使用VMP SDK进行虚拟化，那么在编译这段代码的时候就会使用等效的VM虚拟指令来达到同样的功能。
vPushMem uType
vPushMem lpCaption
vPushMem lpText
vPushMem hWnd,
vCall vCode

注意，虚拟指令的也有自己的机器码，但和原本的x86汇编机器码完全不一样，而且常常是一堆无意义的代码，它们只能由VM虚拟解释器(Dispatcher)来解释并执行(关于虚拟解释器接下来会详细解释)，
所以，我们在用OD等工具进行反汇编分析的时候，看到的就是一大堆的无意义的代码，甚至还有大量的junk code，jmp code等，导致我们无法从反编译层面分析出原本的代码流向，自然也就无法轻易的进行
算法逆向分析了。

我们在逆向虚拟机加壳后的程序中看到的汇编代码其实不是x86汇编代码，而是字节码(伪指令)。它是由指令执行系统定义的一套指令和数据组成的一串数据流。
java的JVM、.NET或其他动态语言的虚拟机都是靠解释字节码来执行的，但它们的字节码(中间语言IL)之间不通用，因为每个系统设计的字节码都是为自己使用的，并不兼容其他的系统。
所以虚拟机的脱壳很难写出一个通用的脱壳机，原则上只要虚拟指令集一变动，那原本的伪指令的解释就会发生变化。
我个人理解要逆向被VM SDK保护起来的原始代码，只有手工分析这段虚拟指令，找到虚拟指令和原始汇编的对应关系，然后重写出原始程序的代码，完成算法的逆向和分析。

这张图是一个虚拟机执行时的整图概述，VStartVM部分初始化虚拟机，VMDispatcher负责调度这些Handler，Handler可以理解为一个个的子函数(功能代码)，它是每一个伪指令
对应的执行功能代码，为什么要出现一条伪指令对应着一个Handler执行模块呢？这和虚拟机加壳的指令膨胀有关，被虚拟机加壳后，同样一条指令被翻译成了虚拟伪指令，
一条虚拟伪指令往往对应着好几倍的等效代码，当中可能还加入了花指令，整个Handler加起来可能就等效为原本的一条x86汇编指令。
Bytecode就是虚拟伪指令，在程序中，VMDispatcher往往是一个类while结构，不断的循环读取伪指令，然后执行。
Virtual Machine Loop Start
...
...
...
--> Decode VM Instruction's
...
--> Execute the Decoded VM Instruction's
...
...
--> Check Special Conditions
...
...
...
Virtual Machine Loop End


在理解Handler(VM虚拟指令和x86汇编的映射)之前，有必要先理解一下VM的启动框架和调用约定，每种代码执行模式都需要有启动框架和调用约定。
在C语言中，在进入main函数之前，会有一些C库添加的启动函数，它们负责对栈区，变量进行初始化，在main函数执行完毕之后再收回控制权，这就叫做启动框架。而
C CALL，StdCall等约定就是调用约定，它们规定了传参方式和堆栈平衡方式。

同样，对与VM虚拟机，我们也需要有一种启动框架和调用约定，来保证虚拟指令的正确执行以及虚拟和现实代码之间的切换。

1. 调度器VStartVM
VStartVM过程将真实环境压入后有一个VMDispatcher标签，当handler执行完毕之后会跳回到这里形成了一个循环，所以VStartVM过程也可以叫做Dispatcher(调度器)
VStartVM首先将所有的寄存器的符号压入堆栈，然后esi指向字节码的起始地址，ebp指向真实的堆栈，edi指向VMContext，esp再减去40h(这个值可以变化)就是VM用的堆栈地址了。
换句话说，这里将VM的环境结构和堆栈都放在了当前堆栈之下200h处的位置上了。
因为堆栈是变化的，在执行完跟堆栈有关的指令时总应该检查一下真实的堆栈是否已经接近自己存放的数据了，如果是，那么再将自己的结构往更地下移动。
然后从 movzx eax, byte ptr [esi]这句开始，读字节码，然后在jump表中寻找相应的handler，并跳转过去继续执行。

VStartVM
    push eax
    push ebx
    push ecx
    push edx
    push esi
    push edi
    push ebp
    pushfd
    mov esi, [esp+0x20] ;字节码开始的地址(寄存器占了32byte，从32byte开始就是刚才push的字节码的位置)
    mov ebp, esp ;ebp就是堆栈了
    sub esp, 0x200
    mov edi, esp ;edi就是VMContext
    sub esp, 0x40 ;这时的esp就是VM用的堆栈了
VMDispatcher
    movzx eax, byte ptr [esi] ;获得bytecode
    lea esi, [esi+1] ;跳过这个字节
    jmp dword ptr [eax*4 + JUMPADDR] ;跳到handler执行处

调用方法
push 指向字节码的起始地址
jmp VStartVM

这里有几个约定
edi = VMContext
esi = 当前字节码地址
ebp = 真实堆栈

在整个虚拟机代码执行过程中，必须要遵守一个事实。
1. 不能将edi,esi,ebp寄存器另做他用
2. edi指向的VMContext存放在栈中而没有存放在其他固定地址或者申请的堆空间中，是因为考虑到多线程程序的兼容



2. 虚拟环境 VMContext
VMContext即虚拟环境结构，存放了一些需要用到的值
struct VMContext
{
    DWORD v_eax;
    DWORD v_ebx;
    DWORD v_ecx;
    DWORD v_edx;
    DWORD v_esi;
    DWORD v_edi;
    DWORD v_ebp;
    DWORD v_efl; 符号寄存器
}


3. 平衡堆栈 VBegin和VCheckEsp
VMStartVM将所有的寄存器都压入了堆栈，所以，首先应该使堆栈平衡才能开始执行真正的代码(这是书上写的)。我的理解是这里要先将现实的代码中的寄存器复制切换到虚拟代码中

Vebing:
    mov eax, dword ptr [ebp]  ;ebp指向真实的堆栈
    mov [edi+0x1C], eax       ;edi指向VMContext
    add esp, 4
    
    mov eax, dword ptr [ebp]  ;ebp指向真实的堆栈
    mov [edi+0x18], eax       ;v_ebp
    add esp, 4

    mov eax, dword ptr [ebp]  ;ebp指向真实的堆栈
    mov [edi+0x14], eax       ;v_edi
    add esp, 4

    mov eax, dword ptr [ebp]  ;ebp指向真实的堆栈
    mov [edi+0x10], eax       ;v_esi
    add esp, 4

    mov eax, dword ptr [ebp]  ;ebp指向真实的堆栈
    mov [edi+0x0C], eax       ;v_edx
    add esp, 4

    mov eax, dword ptr [ebp]  ;ebp指向真实的堆栈
    mov [edi+0x08], eax       ;v_ecx
    add esp, 

    mov eax, dword ptr [ebp]  ;ebp指向真实的堆栈
    mov [edi+0x04], eax       ;v_ebx
    add esp, 4

    mov eax, dword ptr [ebp]  ;ebp指向真实的堆栈
    mov [edi], eax       ;v_eax
    add esp, 4

    add esp, 4        ;释放参数
    jmp VMDispatcher

执行这个"Handler"之后，堆栈就平衡了，就可以开始"继续"执行真正的伪代码了。

还有一个问题，因为将VMContext结构存放在当前使用的堆栈(EBP)更低地址的区域(初始时VMContext距离栈顶EBP有0x200的空间)，当堆栈被push压入数据时，
总会在某条指令之后改写VMContext的内容，为了避免这种情况，就需要对VMContext进行动态移位。


VCheckEsp:
    lea eax, dword ptr [edi+0x100]  ;edi指向VMContext
    cmp eax, ebp     ;小于则继续正常运行
    jl VMDispatcher
            
    mov edx, edi    ;否则，则进行移位
    mov ecx, esp
    sub ecx, edx    ;计算一下需要搬移的字节空间，作为循环复制的次数

    push esi    ;保存ip指针
    mov esi, esp
    sub esp, 0x60
    mov edi, esp
    push edi    ;保存新的edi VMContext基址
    sub esp, 0x40
    
    cld
    rep movsb    ;复制
    pop edi
    pop esi
    jmp VMDispatcher

一些设计到堆栈的Handler在执行后跳到VCheckEsp判断esp是否接近VMContext所在的位置，如果是就将VMContext结构复制到更低的地方去




2. VM虚拟指令和x86汇编的映射原理
这是VM虚拟机的核心部分，即把每条x86指令或每一类x86汇编指令转换为等效的伪指令Handler，可以说，不同的虚拟机都有自己的一套为指令集，不同的为指令集对原始的
x86汇编指令的翻译都是不同的

handler分两大类：
1. 辅助handler，指一些更重要的、更基本的指令，如堆栈指令
2. 普通handler，指用来执行普通的x86指令的指令，如运算指令

1. 辅助handler
辅助handler除了VBegin这些维护虚拟机不会导致崩溃的handler之外，就是专门用来处理堆栈的handler了。

vPushReg32:
    mov eax, dword ptr [esi] ;从字节码中得到VMContext中的寄存器偏移
    add esi, 4
    mov eax, dword ptr [edi+eax] ;得到寄存器的值
    push eax ;压入寄存器
    jmp VMDispatcher

vPushImm32:
    mov eax, dword ptr [esi]
    add esi, 4
    push eax
    jmp VMDispatcher

vPushMem32:
    mov eax, 0
    mov ecx, 0
    mov eax. dword ptr [esp] ;第一个寄存器偏移
    test eax, eax
    cmovge edx, dword ptr [edi+eax] ;如果不是负数则赋值
    mov eax, dword ptr [esp+4] ;第二个寄存器偏移
    test eax, eax
    cmovge ecx, dword ptr [edi+eax] ;如果不是负数则赋值
    imul ecx, dword ptr [esp+8] ;第二个寄存器的乘积
    add ecx, dword ptr [esp+0x0C] ;第三个为内存地址常量
    add edx, ecx
    add esp, 0x10 ;释放参数
    push edx ;插入参数
    jmp VMDispatcher

vPopReg32:
    mov eax, dword ptr [esi] ;得到reg偏移
    add esi, 4
    pop dword ptr [edi+eax] ;弹回寄存器
    jmp VMDispatcher

vFree:
    add esp, 4
    jmp VMDispatcher


2. 普通Handler

add指令的形式通常有 
add reg,imm
add reg,reg
add reg,mem
add mem,reg
等写法....
如果将操作数都先交给堆栈handler处理，那么执行到vadd handler时，已经是一个立即数存在堆栈中了，vadd handler不必去管它从哪里来，只需要用这个立即数做加法操作即可。
也就是说，将辅助指令和普通指令配合起来来一起完成x86指令到伪指令的转换

vadd:
    mov eax, [esp+4] ;取源操作数
    mov ebx, [esp] ;取目的操作数
    add ebx, eax
    add esp, 8 ;平衡堆栈
    push ebx ;压入堆栈


下面的指令转换为伪代码:
add esi, eax
转换为
vPushReg32 eax_index    ;eax在VMContext下的偏移
vPushReg32 esi_index
vadd
vPopReg32 esi_index

add esi, 1234
转换为
vPushImm32 1234
vPushReg32 esi_index
vadd
vPopReg32 esi_index


add esi, dword ptr [401000]
转换为
vPushImm32 401000
vPushImm32 -1 ;scale
vPushImm32 -1 ;reg_index
vPushImm32 -1 ;reg2_index
vPushMem32 ;压入内存地址的值
vPushReg32 esi_index
vadd
vPopReg32 esi_index

这就是add指令的多种实现，我们可以发现无论是哪一种形式，都可以使用vadd来执行，只是使用了不同的堆栈handler，这就是Stack_Based虚拟机的方便之处。



标志位的问题：
在相关的handler执行前恢复标志位，执行后保存标志位。以stc命令来说，stc指令是将标志的CF位置为1
VStc:
    push [edi+0x1C]
    popfd
    stc
    pushfd
    pop [edi+0x1C]
    jmp VMDispatcher

这样操作之后就能保证代码中的标志不会被虚拟机引擎所执行的代码所改变



转移指令：
转移指令有条件转移、无条件转移、call和retn
实现时可以将esi指向当前字节码的地址，esi指针就好比真实CPU中的eip寄存器，可以通过改写esi寄存器的值来更改流程。无条件跳转jmp的handler比较简单

vJmp:
    mov esi, dword ptr [esp] ;[esp]指向要跳转到的地址
    add esp, 4
    jmp VMDispatcher

条件转移jcc指令稍微有一点麻烦，因为它要通过测试标志位来判断视为需要更改流程


基本上所有条件跳转指令都有相应的CMOV指令来匹配。
vJne:
    cmovne esi, [esp]
    add esp, 4
    jmp VMDispatcher

vJa:
    cmova esi, [esp]
    add esp, 4
    jmp VMDispatcher

vJae:
    cmovae esi, [esp]
    add esp, 4
    jmp VMDispatcher

vJb:
    cmovb esi, [esp]
    add esp, 4
    jmp VMDispatcher

vJbe:
    cmovbe esi, [esp]
    add esp, 4
    jmp VMDispatcher

je:
    cmove esi, [esp]
    add esp, 4
    jmp VMDispatcher

jg:
    cmovg esi, [esp]
    add esp, 4
    jmp VMDispatcher


call指令：
call和retn指令虽然也是转移指令，但是它们的功能不太一样。
首先，虚拟机设计为只在一个堆栈层次上运行

    mov eax, 1234
    push 1234
    call anotherfunc
theNext:
    add esp, 4

其中第1、2、4条指令都是在当前堆栈层次上执行的，而call anotherfunc是调用子函数，会将控制权移交给另外的代码，这些代码是不受虚拟机控制的。所以碰到call指令，必须退出虚拟机，让子函数在真实CPU中执行完毕后再交回给虚拟机执行下一条指令。
vcall:
    push theNext
    jmp anotherfunc

如果想在推出虚拟机后让anotherfunc这个函数返回后再次拿回控制权，可以更改返回地址，来达到继续接管代码的操作。在一个地址上写上这样的代码:
theNextVM:
    push theNextByteCode
    jmp VStartVM

这是一个重新进入虚拟机的代码，theNextByteCode代表了theNext之后的代码字节码。只需将theNext的地址改为theNextVM的地址，即可完美地模拟call指令了。当虚拟机外部的代码执行完毕后ret回来的时候就会执行theNextVM的代码，从而使虚拟机继续接管控制权。

vcall:
    push all vreg ;所有虚拟寄存器
    poo all reg ;弹出到真实寄存器中
    push 返回地址 ;
    push 要调用的函数的地址
    retn


ret指令：
retn指令和其他普通指令不太一样，retn在这里被虚拟机认为是一个推出函数。retn有两种写法:一种是不带操作数的，另一种是带操作数的。
retn
retn 4
第一种retn形式先得到当前esp中存放的返回地址，然后再释放返回地址的堆栈并跳转到返回地址
第二种在释放返回地址的堆栈时再释放操作数的空间

vRetn:
    xor eax, eax
    mov ax, word ptr [esi] ;retn的操作数是word型的，所以最大只有0xFFFF
    add esi, 2
    mov ebx, dword ptr [ebp] ;得到要返回的地址
    add esp, 4 ;释放空间
    add esp, eax ;如果有操作数，同样释放
    push ebx ;压入返回地址
    push ebp ;压入堆栈指针
    push [edi+0x1C]
    push [edi+0x18]
    push [edi+0x14]
    push [edi+0x10]
    push [edi+0x0C]
    push [edi+0x08]
    push [edi+0x04] 
    push [edi]
    pop eax
    pop ebx
    pop ecx
    pop edx
    pop esi
    pop edi
    pop ebp
    popfd
    pop esp ;还原堆栈指针到esp中，而VM_CONTEXT也算是自动销毁了
    retn



不可识别指令:
在这里任何不能识别的指令都可将其划分为不可模拟指令，碰到这类指令时，只能与vcall使用相同的方法，即先退出虚拟机，执行这个指令，
然后再压入下一字节码(虚拟指令)的地址，重新进入虚拟机。






3. 用VM虚拟指令构造一段小程序
上面阐述了一些关于虚拟机运行和虚拟指令的一些原理和理解，接下来从实践的调度来实现一个最简单的基于虚拟指令的小程序。
即我们的执行代码是我们自定义的一些伪指令，调度器在执行的时候不断的从伪指令字节码中取指令，然后到Handler地址表中寻址对应的执行体Handler，进行执行，执行完毕后
再跳回到调度器回收控制权，依次循环，知道执行完所有的虚拟指令。

------------------------------
code:

#include "stdafx.h"
#include "windows.h"

/* 下面是虚拟指令 我们只模拟了2条指令 */

//push 0x12345678  push一个4字节的数
#define vPushData    0x10

//call 0x12345678  call一个4字节的地址
#define vCall        0x12

//结束符
#define vEnd        0xff

//一个字符串
char *str = "Hello World";

/*
    这是我们构造的虚拟指令, 数据还不  在mian里面我们进行了修改
    push 0
    push offset str
    push offset str  ;把字符串的地址入栈
    push 0
    call MessageBoxA ;
*/
BYTE bVmData[] = {    vPushData,    0x00, 0x00, 0x00,0x00, 
                    vPushData,    0x00, 0x00, 0x00,0x00,
                    vPushData,    0x00, 0x00, 0x00, 0x00,
                    vPushData,    0x00, 0x00, 0x00,0x00,
                    vCall,    0x00, 0x00, 0x00, 0x00,
                    vEnd};


//这就是简单的虚拟引擎了
_declspec(naked) void  VM(PVOID pvmData)
{
    __asm
    {
        //取vCode地址放入ecx
        mov ecx, dword ptr ss:[esp+4]
__vstart:
        //取第一个字节到al中
        mov al, byte ptr ds:[ecx]
        cmp al, vPushData
        je    __vPushData
        cmp al, vCall
        je    __vCall
        cmp al, vEnd
        je __vEnd
        int 3
__vPushData:
        inc ecx
        mov edx, dword ptr ds:[ecx]
        push edx
        add ecx, 4
        jmp __vstart
__vCall:
        inc ecx
        mov edx, dword ptr ds:[ecx]
        call edx
        add ecx, 4
        jmp __vstart
__vEnd:
        ret
    }
}


int main(int argc, char* argv[])
{
    //修改虚拟指令的数据

　　*(DWORD *)(bVmData+5 + 1) = (DWORD)str;
    *(DWORD *)(bVmData+10 + 1) = (DWORD)str;
    *(DWORD *)(bVmData+20 + 1) = (DWORD)MessageBoxA;
    
    //执行虚拟指令
    VM(bVmData);
    return 0;
}

这个程序中的__vstart就相当于前面说的VMDispatcher，对伪指令进行识别判断，采取调度策略使程序流跳转到相应的Handler里去。
每个Handler在执行完之后都有一句相同的代码：jmp __vstart 用于回收控制权到VMDispatcher中。以便下一次调度。
最后，当所有的伪指令都执行完之后，程序识别到vEnd，就ret退出程序。

基本上，这个虚拟机的原理就是这样了。




4. 总结
这里有一点关键点其实没有弄清楚，就是x86汇编指令到虚拟机伪指令的转换问题，上面的小程序我们是直接自定义了一套伪指令和映射规则，实际情况中如VMProtect加壳软件要
解决的是根据原本的汇编指令动态的转换为伪指令再回写到原程序的二进制文件中。

还有一个问题没搞明白的就是，如果要对虚拟机的程序进行逆向分析或脱壳。我的理解就只能是想办法找到目标程序伪指令和x86汇编之间的映射关系，然后手工分析这段代码
(因为考虑到效率问题，一般的程序都是只对一些核心算法进行了虚拟化)，将这段代码重写出来，以达到逆向分析或脱壳的目的。

不知道理解的对不对，逆向这块感觉算法分析和虚拟机是个难点，以后可以针对这个问题进行一些更xxxxxxxxxx push uTypepush lpCaptionpush lpTextpush hWnd,call MessageBox这是一段x86的汇编指令，编译器在翻译的时候会产生一个固定模式的汇编代码(在同一个CPU指令集下)。但如果我们对原本的C代码使用VMP SDK进行虚拟化，那么在编译这段代码的时候就会使用等效的VM虚拟指令来达到同样的功能。vPushMem uTypevPushMem lpCaptionvPushMem lpTextvPushMem hWnd,vCall vCode注意，虚拟指令的也有自己的机器码，但和原本的x86汇编机器码完全不一样，而且常常是一堆无意义的代码，它们只能由VM虚拟解释器(Dispatcher)来解释并执行(关于虚拟解释器接下来会详细解释)，所以，我们在用OD等工具进行反汇编分析的时候，看到的就是一大堆的无意义的代码，甚至还有大量的junk code，jmp code等，导致我们无法从反编译层面分析出原本的代码流向，自然也就无法轻易的进行算法逆向分析了。我们在逆向虚拟机加壳后的程序中看到的汇编代码其实不是x86汇编代码，而是字节码(伪指令)。它是由指令执行系统定义的一套指令和数据组成的一串数据流。java的JVM、.NET或其他动态语言的虚拟机都是靠解释字节码来执行的，但它们的字节码(中间语言IL)之间不通用，因为每个系统设计的字节码都是为自己使用的，并不兼容其他的系统。所以虚拟机的脱壳很难写出一个通用的脱壳机，原则上只要虚拟指令集一变动，那原本的伪指令的解释就会发生变化。我个人理解要逆向被VM SDK保护起来的原始代码，只有手工分析这段虚拟指令，找到虚拟指令和原始汇编的对应关系，然后重写出原始程序的代码，完成算法的逆向和分析。这张图是一个虚拟机执行时的整图概述，VStartVM部分初始化虚拟机，VMDispatcher负责调度这些Handler，Handler可以理解为一个个的子函数(功能代码)，它是每一个伪指令对应的执行功能代码，为什么要出现一条伪指令对应着一个Handler执行模块呢？这和虚拟机加壳的指令膨胀有关，被虚拟机加壳后，同样一条指令被翻译成了虚拟伪指令，一条虚拟伪指令往往对应着好几倍的等效代码，当中可能还加入了花指令，整个Handler加起来可能就等效为原本的一条x86汇编指令。Bytecode就是虚拟伪指令，在程序中，VMDispatcher往往是一个类while结构，不断的循环读取伪指令，然后执行。Virtual Machine Loop Start.........--> Decode VM Instruction's...--> Execute the Decoded VM Instruction's......--> Check Special Conditions.........Virtual Machine Loop End在理解Handler(VM虚拟指令和x86汇编的映射)之前，有必要先理解一下VM的启动框架和调用约定，每种代码执行模式都需要有启动框架和调用约定。在C语言中，在进入main函数之前，会有一些C库添加的启动函数，它们负责对栈区，变量进行初始化，在main函数执行完毕之后再收回控制权，这就叫做启动框架。而C CALL，StdCall等约定就是调用约定，它们规定了传参方式和堆栈平衡方式。同样，对与VM虚拟机，我们也需要有一种启动框架和调用约定，来保证虚拟指令的正确执行以及虚拟和现实代码之间的切换。1. 调度器VStartVMVStartVM过程将真实环境压入后有一个VMDispatcher标签，当handler执行完毕之后会跳回到这里形成了一个循环，所以VStartVM过程也可以叫做Dispatcher(调度器)VStartVM首先将所有的寄存器的符号压入堆栈，然后esi指向字节码的起始地址，ebp指向真实的堆栈，edi指向VMContext，esp再减去40h(这个值可以变化)就是VM用的堆栈地址了。换句话说，这里将VM的环境结构和堆栈都放在了当前堆栈之下200h处的位置上了。因为堆栈是变化的，在执行完跟堆栈有关的指令时总应该检查一下真实的堆栈是否已经接近自己存放的数据了，如果是，那么再将自己的结构往更地下移动。然后从 movzx eax, byte ptr [esi]这句开始，读字节码，然后在jump表中寻找相应的handler，并跳转过去继续执行。VStartVM    push eax    push ebx    push ecx    push edx    push esi    push edi    push ebp    pushfd    mov esi, [esp+0x20] ;字节码开始的地址(寄存器占了32byte，从32byte开始就是刚才push的字节码的位置)    mov ebp, esp ;ebp就是堆栈了    sub esp, 0x200    mov edi, esp ;edi就是VMContext    sub esp, 0x40 ;这时的esp就是VM用的堆栈了VMDispatcher    movzx eax, byte ptr [esi] ;获得bytecode    lea esi, [esi+1] ;跳过这个字节    jmp dword ptr [eax*4 + JUMPADDR] ;跳到handler执行处调用方法push 指向字节码的起始地址jmp VStartVM这里有几个约定edi = VMContextesi = 当前字节码地址ebp = 真实堆栈在整个虚拟机代码执行过程中，必须要遵守一个事实。1. 不能将edi,esi,ebp寄存器另做他用2. edi指向的VMContext存放在栈中而没有存放在其他固定地址或者申请的堆空间中，是因为考虑到多线程程序的兼容2. 虚拟环境 VMContextVMContext即虚拟环境结构，存放了一些需要用到的值struct VMContext{    DWORD v_eax;    DWORD v_ebx;    DWORD v_ecx;    DWORD v_edx;    DWORD v_esi;    DWORD v_edi;    DWORD v_ebp;    DWORD v_efl; 符号寄存器}3. 平衡堆栈 VBegin和VCheckEspVMStartVM将所有的寄存器都压入了堆栈，所以，首先应该使堆栈平衡才能开始执行真正的代码(这是书上写的)。我的理解是这里要先将现实的代码中的寄存器复制切换到虚拟代码中Vebing:    mov eax, dword ptr [ebp]  ;ebp指向真实的堆栈    mov [edi+0x1C], eax       ;edi指向VMContext    add esp, 4        mov eax, dword ptr [ebp]  ;ebp指向真实的堆栈    mov [edi+0x18], eax       ;v_ebp    add esp, 4    mov eax, dword ptr [ebp]  ;ebp指向真实的堆栈    mov [edi+0x14], eax       ;v_edi    add esp, 4    mov eax, dword ptr [ebp]  ;ebp指向真实的堆栈    mov [edi+0x10], eax       ;v_esi    add esp, 4    mov eax, dword ptr [ebp]  ;ebp指向真实的堆栈    mov [edi+0x0C], eax       ;v_edx    add esp, 4    mov eax, dword ptr [ebp]  ;ebp指向真实的堆栈    mov [edi+0x08], eax       ;v_ecx    add esp,     mov eax, dword ptr [ebp]  ;ebp指向真实的堆栈    mov [edi+0x04], eax       ;v_ebx    add esp, 4    mov eax, dword ptr [ebp]  ;ebp指向真实的堆栈    mov [edi], eax       ;v_eax    add esp, 4    add esp, 4        ;释放参数    jmp VMDispatcher执行这个"Handler"之后，堆栈就平衡了，就可以开始"继续"执行真正的伪代码了。还有一个问题，因为将VMContext结构存放在当前使用的堆栈(EBP)更低地址的区域(初始时VMContext距离栈顶EBP有0x200的空间)，当堆栈被push压入数据时，总会在某条指令之后改写VMContext的内容，为了避免这种情况，就需要对VMContext进行动态移位。VCheckEsp:    lea eax, dword ptr [edi+0x100]  ;edi指向VMContext    cmp eax, ebp     ;小于则继续正常运行    jl VMDispatcher                mov edx, edi    ;否则，则进行移位    mov ecx, esp    sub ecx, edx    ;计算一下需要搬移的字节空间，作为循环复制的次数    push esi    ;保存ip指针    mov esi, esp    sub esp, 0x60    mov edi, esp    push edi    ;保存新的edi VMContext基址    sub esp, 0x40        cld    rep movsb    ;复制    pop edi    pop esi    jmp VMDispatcher一些设计到堆栈的Handler在执行后跳到VCheckEsp判断esp是否接近VMContext所在的位置，如果是就将VMContext结构复制到更低的地方去2. VM虚拟指令和x86汇编的映射原理这是VM虚拟机的核心部分，即把每条x86指令或每一类x86汇编指令转换为等效的伪指令Handler，可以说，不同的虚拟机都有自己的一套为指令集，不同的为指令集对原始的x86汇编指令的翻译都是不同的handler分两大类：1. 辅助handler，指一些更重要的、更基本的指令，如堆栈指令2. 普通handler，指用来执行普通的x86指令的指令，如运算指令1. 辅助handler辅助handler除了VBegin这些维护虚拟机不会导致崩溃的handler之外，就是专门用来处理堆栈的handler了。vPushReg32:    mov eax, dword ptr [esi] ;从字节码中得到VMContext中的寄存器偏移    add esi, 4    mov eax, dword ptr [edi+eax] ;得到寄存器的值    push eax ;压入寄存器    jmp VMDispatchervPushImm32:    mov eax, dword ptr [esi]    add esi, 4    push eax    jmp VMDispatchervPushMem32:    mov eax, 0    mov ecx, 0    mov eax. dword ptr [esp] ;第一个寄存器偏移    test eax, eax    cmovge edx, dword ptr [edi+eax] ;如果不是负数则赋值    mov eax, dword ptr [esp+4] ;第二个寄存器偏移    test eax, eax    cmovge ecx, dword ptr [edi+eax] ;如果不是负数则赋值    imul ecx, dword ptr [esp+8] ;第二个寄存器的乘积    add ecx, dword ptr [esp+0x0C] ;第三个为内存地址常量    add edx, ecx    add esp, 0x10 ;释放参数    push edx ;插入参数    jmp VMDispatchervPopReg32:    mov eax, dword ptr [esi] ;得到reg偏移    add esi, 4    pop dword ptr [edi+eax] ;弹回寄存器    jmp VMDispatchervFree:    add esp, 4    jmp VMDispatcher2. 普通Handleradd指令的形式通常有 add reg,immadd reg,regadd reg,memadd mem,reg等写法....如果将操作数都先交给堆栈handler处理，那么执行到vadd handler时，已经是一个立即数存在堆栈中了，vadd handler不必去管它从哪里来，只需要用这个立即数做加法操作即可。也就是说，将辅助指令和普通指令配合起来来一起完成x86指令到伪指令的转换vadd:    mov eax, [esp+4] ;取源操作数    mov ebx, [esp] ;取目的操作数    add ebx, eax    add esp, 8 ;平衡堆栈    push ebx ;压入堆栈下面的指令转换为伪代码:add esi, eax转换为vPushReg32 eax_index    ;eax在VMContext下的偏移vPushReg32 esi_indexvaddvPopReg32 esi_indexadd esi, 1234转换为vPushImm32 1234vPushReg32 esi_indexvaddvPopReg32 esi_indexadd esi, dword ptr [401000]转换为vPushImm32 401000vPushImm32 -1 ;scalevPushImm32 -1 ;reg_indexvPushImm32 -1 ;reg2_indexvPushMem32 ;压入内存地址的值vPushReg32 esi_indexvaddvPopReg32 esi_index这就是add指令的多种实现，我们可以发现无论是哪一种形式，都可以使用vadd来执行，只是使用了不同的堆栈handler，这就是Stack_Based虚拟机的方便之处。标志位的问题：在相关的handler执行前恢复标志位，执行后保存标志位。以stc命令来说，stc指令是将标志的CF位置为1VStc:    push [edi+0x1C]    popfd    stc    pushfd    pop [edi+0x1C]    jmp VMDispatcher这样操作之后就能保证代码中的标志不会被虚拟机引擎所执行的代码所改变转移指令：转移指令有条件转移、无条件转移、call和retn实现时可以将esi指向当前字节码的地址，esi指针就好比真实CPU中的eip寄存器，可以通过改写esi寄存器的值来更改流程。无条件跳转jmp的handler比较简单vJmp:    mov esi, dword ptr [esp] ;[esp]指向要跳转到的地址    add esp, 4    jmp VMDispatcher条件转移jcc指令稍微有一点麻烦，因为它要通过测试标志位来判断视为需要更改流程基本上所有条件跳转指令都有相应的CMOV指令来匹配。vJne:    cmovne esi, [esp]    add esp, 4    jmp VMDispatchervJa:    cmova esi, [esp]    add esp, 4    jmp VMDispatchervJae:    cmovae esi, [esp]    add esp, 4    jmp VMDispatchervJb:    cmovb esi, [esp]    add esp, 4    jmp VMDispatchervJbe:    cmovbe esi, [esp]    add esp, 4    jmp VMDispatcherje:    cmove esi, [esp]    add esp, 4    jmp VMDispatcherjg:    cmovg esi, [esp]    add esp, 4    jmp VMDispatchercall指令：call和retn指令虽然也是转移指令，但是它们的功能不太一样。首先，虚拟机设计为只在一个堆栈层次上运行    mov eax, 1234    push 1234    call anotherfunctheNext:    add esp, 4其中第1、2、4条指令都是在当前堆栈层次上执行的，而call anotherfunc是调用子函数，会将控制权移交给另外的代码，这些代码是不受虚拟机控制的。所以碰到call指令，必须退出虚拟机，让子函数在真实CPU中执行完毕后再交回给虚拟机执行下一条指令。vcall:    push theNext    jmp anotherfunc如果想在推出虚拟机后让anotherfunc这个函数返回后再次拿回控制权，可以更改返回地址，来达到继续接管代码的操作。在一个地址上写上这样的代码:theNextVM:    push theNextByteCode    jmp VStartVM这是一个重新进入虚拟机的代码，theNextByteCode代表了theNext之后的代码字节码。只需将theNext的地址改为theNextVM的地址，即可完美地模拟call指令了。当虚拟机外部的代码执行完毕后ret回来的时候就会执行theNextVM的代码，从而使虚拟机继续接管控制权。vcall:    push all vreg ;所有虚拟寄存器    poo all reg ;弹出到真实寄存器中    push 返回地址 ;    push 要调用的函数的地址    retnret指令：retn指令和其他普通指令不太一样，retn在这里被虚拟机认为是一个推出函数。retn有两种写法:一种是不带操作数的，另一种是带操作数的。retnretn 4第一种retn形式先得到当前esp中存放的返回地址，然后再释放返回地址的堆栈并跳转到返回地址第二种在释放返回地址的堆栈时再释放操作数的空间vRetn:    xor eax, eax    mov ax, word ptr [esi] ;retn的操作数是word型的，所以最大只有0xFFFF    add esi, 2    mov ebx, dword ptr [ebp] ;得到要返回的地址    add esp, 4 ;释放空间    add esp, eax ;如果有操作数，同样释放    push ebx ;压入返回地址    push ebp ;压入堆栈指针    push [edi+0x1C]    push [edi+0x18]    push [edi+0x14]    push [edi+0x10]    push [edi+0x0C]    push [edi+0x08]    push [edi+0x04]     push [edi]    pop eax    pop ebx    pop ecx    pop edx    pop esi    pop edi    pop ebp    popfd    pop esp ;还原堆栈指针到esp中，而VM_CONTEXT也算是自动销毁了    retn不可识别指令:在这里任何不能识别的指令都可将其划分为不可模拟指令，碰到这类指令时，只能与vcall使用相同的方法，即先退出虚拟机，执行这个指令，然后再压入下一字节码(虚拟指令)的地址，重新进入虚拟机。3. 用VM虚拟指令构造一段小程序上面阐述了一些关于虚拟机运行和虚拟指令的一些原理和理解，接下来从实践的调度来实现一个最简单的基于虚拟指令的小程序。即我们的执行代码是我们自定义的一些伪指令，调度器在执行的时候不断的从伪指令字节码中取指令，然后到Handler地址表中寻址对应的执行体Handler，进行执行，执行完毕后再跳回到调度器回收控制权，依次循环，知道执行完所有的虚拟指令。------------------------------code:#include "stdafx.h"#include "windows.h"/* 下面是虚拟指令 我们只模拟了2条指令 *///push 0x12345678  push一个4字节的数#define vPushData    0x10//call 0x12345678  call一个4字节的地址#define vCall        0x12//结束符#define vEnd        0xff//一个字符串char *str = "Hello World";/*    这是我们构造的虚拟指令, 数据还不  在mian里面我们进行了修改    push 0    push offset str    push offset str  ;把字符串的地址入栈    push 0    call MessageBoxA ;*/BYTE bVmData[] = {    vPushData,    0x00, 0x00, 0x00,0x00,                     vPushData,    0x00, 0x00, 0x00,0x00,                    vPushData,    0x00, 0x00, 0x00, 0x00,                    vPushData,    0x00, 0x00, 0x00,0x00,                    vCall,    0x00, 0x00, 0x00, 0x00,                    vEnd};//这就是简单的虚拟引擎了_declspec(naked) void  VM(PVOID pvmData){    __asm    {        //取vCode地址放入ecx        mov ecx, dword ptr ss:[esp+4]__vstart:        //取第一个字节到al中        mov al, byte ptr ds:[ecx]        cmp al, vPushData        je    __vPushData        cmp al, vCall        je    __vCall        cmp al, vEnd        je __vEnd        int 3__vPushData:        inc ecx        mov edx, dword ptr ds:[ecx]        push edx        add ecx, 4        jmp __vstart__vCall:        inc ecx        mov edx, dword ptr ds:[ecx]        call edx        add ecx, 4        jmp __vstart__vEnd:        ret    }}int main(int argc, char* argv[]){    //修改虚拟指令的数据　　*(DWORD *)(bVmData+5 + 1) = (DWORD)str;    *(DWORD *)(bVmData+10 + 1) = (DWORD)str;    *(DWORD *)(bVmData+20 + 1) = (DWORD)MessageBoxA;        //执行虚拟指令    VM(bVmData);    return 0;}这个程序中的__vstart就相当于前面说的VMDispatcher，对伪指令进行识别判断，采取调度策略使程序流跳转到相应的Handler里去。每个Handler在执行完之后都有一句相同的代码：jmp __vstart 用于回收控制权到VMDispatcher中。以便下一次调度。最后，当所有的伪指令都执行完之后，程序识别到vEnd，就ret退出程序。基本上，这个虚拟机的原理就是这样了。4. 总结这里有一点关键点其实没有弄清楚，就是x86汇编指令到虚拟机伪指令的转换问题，上面的小程序我们是直接自定义了一套伪指令和映射规则，实际情况中如VMProtect加壳软件要解决的是根据原本的汇编指令动态的转换为伪指令再回写到原程序的二进制文件中。还有一个问题没搞明白的就是，如果要对虚拟机的程序进行逆向分析或脱壳。我的理解就只能是想办法找到目标程序伪指令和x86汇编之间的映射关系，然后手工分析这段代码(因为考虑到效率问题，一般的程序都是只对一些核心算法进行了虚拟化)，将这段代码重写出来，以达到逆向分析或脱壳的目的。不知道理解的对不对，逆向这块感觉算法分析和虚拟机是个难点，以后可以针
```