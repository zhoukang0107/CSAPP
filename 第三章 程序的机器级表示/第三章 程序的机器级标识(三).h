3.7 过程（函数 / 方法调用过程）
    要提供对过程的机器级支持，必须要处理许多不同的属性。为了讨论方便，假设过程P调用过程Q，Q执行后返回到P。这些动作包括下面一个或多个机制:
    传递控制:在进入过程Q的时候，程序计数器必须被设置为Q的代码的起始地址，然后在返回时，要把程序计数器设置为P中调用Q后面那条指令的地址。
    传递数据:P必须能够向Q提供一个或多个参数，Q必须能够向P返回一个值。
    分配和释放内存:在开始时，Q可能需要为局部变量分配空间，而在返回前，又必须释放这些存储空间。

3.7.1 运行时栈
    C语言过程调用机制的一个关键特性(大多数其他语言也是如此)在于使用了栈数据结构提供的后进先出的内存管理原则。在过程P调用过程Q的例子中，可以看到当Q在执行时，
P以及所有在向上追溯到P的调用链中的过程，都是暂时被挂起的。当Q运行时，它只需要为局部变量分配新的存储空间，或者设置到另一个过程的调用。另一方面，当Q返回时，任何
它所分配的局部存储空间都可以被释放。因此，程序可以用栈来管理它的过程所需要的存储空间，栈和程序寄存器存放着传递控制和数据、分配内存所需要的信息。当P调用Q时，
控制和数据信息添加到栈尾。当P返回时，这些信息会释放掉。
    x86-64的栈向低地址方向增长，而栈指针%rsp指向栈顶元素。可以用pushq和popq指令将数据存人栈中或是从栈中取出。将栈指针减小一个适当的量可以为没有指定初始值的
数据在栈上分配空间。类似地，可以通过增加栈指针来释放空间。
    当x86-64过程需要的存储空间超出寄存器能够存放的大小时，就会在栈上分配空间。这个部分称为过程的栈帧(stack fram)。当前正在执行的过程的帧总是在栈顶。当过程P
调用过程Q时，会把返回地址压入栈中，指明当Q返回时，要从P程序的哪个位置继续执行。我们把这个返回地址当做P的栈帧的一部分，因为它存放的是与P相关的状态。Q的代码会扩
展当前栈的边界，分配它的栈帧所需的空间。在这个空间中，它可以保存寄存器的值，分配局部变量空间，为它调用的过程设置参数。大多数过程的栈帧都是定长的，在过程的开始
就分配好了。但是有些过程需要变长的帧。通过寄存器，过程P可以传递最多6个整数值(也就是指针和整数)，但是如果Q需要更多的参数，P可以在调用Q之前在自己的栈帧里存储好
这些参数。
    为了提高空间和时间效率，x86-64过程只分配自己所需要的栈帧部分。例如，许多过程有6个或者更少的参数，那么所有的参数都可以通过寄存器传递。实际上，许多函数甚至
根本不需要栈帧。当所有的局部变量都可以保存在寄存器中，而且该函数不会调用任何其他函数(有时称之为叶子过程，此时把过程调用看做树结构)时，就可以这样处理。例如，到
目前为止我们仔细审视过的所有函数都不需要栈帧。

3.7.2 转移控制
    将控制从函数P转移到函数Q只需要简单地把程序计数器(PC)设置为Q的代码的起始位置。不过，当稍后从Q返回的时候，处理器必须记录好它需要继续P的执行的代码位置。在
x86-64机器中，这个信息是用指令call Q调用过程Q来记录的。该指令会把地址A压人栈中，并将PC设置为Q的起始地址。压人的地址A被称为返回地址，是紧跟在call指令后面的那
条指令的地址。对应的指令ret会从栈中弹出地址A，并把PC设置为A.
    call指令有一个目标，即指明被调用过程起始的指令地址。同跳转一样，调用可以是直接的，也可以是间接的。在汇编代码中，直接调用的目标是一个标号，而间接调用的目标
是*后面跟一个操作数指示符.
    图3-27a给出了两个函数top和工eaf的反汇编代码，以及main函数中调用top处的代码。每条指令都以标号标出:L1~L2
(leaf中)，Tl~T4(main中)和M1~M2(main中)。该图的b部分给出了这段代码执
        Disassembly of leaf (long y) y in %rdi)
1   0000000000400540 <leaf>:
2     400540:48 8d 47 02                lea        0x2(%rdi)，%rax    L1:y+2
3     400544:c3                         retq                          L2:Return

4   0000000000400545 <top>:
        Disassembly of top(long x) x in %rdi
      400545:48 83 ef  05               sub        $0x5,%rdi          T1:x-5
      400549:e8 f2 ff ff ff             callq      400540 <leaf>      T2:Call leaf(x-5)
      40054e:48 01 c0                   add        %rax,%rax          T3:Double result
      400551:c3                         retq                          T4:Return
      
        Call to top from function main 
      40055b:e8 e5 ff ff ff             callq      400545 <top>       M1:Call  top (100)
      400560:48 89 c2                   mov        %rax,%rdx          M2:Resume
        a)说明过程调用和返回的反汇编代码
图3-27 包含过程调用和返回的程序的执行细节。使用栈来存储返回地址
          使得能够返回到过程中正确的位置
┌──────────────────────┬────────────────────────────────────────────┬──────────────┐
│指令                   │状态值(指令执行前)                                          
├──────────────────────┼────────────────────────────────────────────┤     描述       
│标号PC指令               %rdi   %rax        orsp         *%rsp                     
├──────────────────────┼────────────────────────────────────────────┼──────────────┤
│M1    Ox90055b   callq│ 100     一    Ox7fffffffe820      一         调用top (100) 
│T1    0x400555   sub  │ 100     一    Ox7fffffffe818   0x400560    │ 进人top       
│T2    0x400559   callq│  95     一    Ox7fffffffe818   0x400560    │ 调用leaf(95)  
│LI    0x400590   lea  │  95     一    Ox7fffffffe810   Ox40059e    │ 进人leaf      
│L2    0x900544   retq │ 一      97    Ox7fffffffe810   Ox40054e    │ 从leaf返回97  
│T3    Ox40054e   add  │ 一      97    Ox7fffffffe818   0x400560    │ 继续top     
│T4    0x400551   retq │ 一      194   Ox7fffffffe818   0x400560    │ 从top返回194  
│M2    0x400560   mov  │ 一      194   Ox7fffffffe820      一         继续main      
└──────────────────────┴────────────────────────────────────────────┴──────────────┘
          b)示例代码的执行过程

3.7.3 数据传送
    当调用一个过程时，除了要把控制传递给它并在过程返回时再传递回来之外，过程调用还可能包括把数据作为参数传递，而从过程返回还有可能包括返回一个值。x86-64中，
大部分过程间的数据传送是通过寄存器实现的。
    x86-64中，可以通过寄存器最多传递6个整型(例如整数和指针)参数。寄存器的使用是有特殊顺序的，寄存器使用的名字取决于要传递的数据类型的大小，如图3-28所示。会
根据参数在参数列表中的顺序为它们分配寄存器。可以通过64位寄存器适当的部分访问小于64位的参数。例如，如果第一个参数是32位的，那么可以用%edi来访问它。
┌──────────────┬─────────────────────────────────────┐
│操作数大小(位) │参数数量                              │
│              ├──────┬──────┬────┬────┬──────┬──────┤
│              │1     │2     │3   │4   │5     │6     │
├──────────────┼──────┼──────┼────┼────┼──────┼──────┤
│64            │%rdi  │%rsi  │%rdx│%rcx│%r8   │%r9   │
├──────────────┼──────┼──────┼────┼────┼──────┼──────┤
│32            │%edi  │%esi  │%edx│%ecx│%r8d  │%r9gd │
├──────────────┼──────┼──────┼────┼────┼──────┼──────┤
│16            │%di   │%si   │%dx │%cx │$r8w  │%r9w  │
├──────────────┼──────┼──────┼────┼────┼──────┼──────┤
│8             │%di1  │%sil  │%d1 │%c1 │%r8b  │%r9b  │
└──────────────┴──────┴──────┴────┴────┴──────┴──────┘
图3-28 传递函数参数的寄存器。寄存器是按照特殊顺序来使用的，
          而使用的名字是根据参数的大小来确定的
    
    如果一个函数有大于6个整型参数，超出6个的部分就要通过栈来传递。假设过程P调用过程Q，有n个整型参数，且n>6。那么P的代码分配的栈帧必须要能假设过程容纳7到n号参
数的存储空间，如图3-25所示。要把参数1~6复制到对应的寄存器，把参数7~n放到栈上，而参数7位于栈顶。通过栈传递参数时，所有的数据大小都向8的倍数对齐。参数到位以后，
程序就可以执行call指令将控制转移到过程Q了。过程Q可以通过寄存器访问参数，有必要的话也可以通过栈访问。
    作为参数传递的示例，考虑图3-2 9 a所示的C函数proc。这个函数有8个参数，包括字节数不同的整数(8, 4, 2和1)和不同类型的指针，每个都是8字节的。
void proc(long  al, long  *alp,
          int   a2, int   *a2p,
          short a3, short *a3p,
          char  a4, char  *a4p)
{
    *a1p+=al;
    *a2p+=a2;
    *a3p+=a3;
    *a4p+=a4;
}
    a)C代码

        void proc(al，aip, a2, a2p, a3, a3p, a4, a4p)
        Arguments passed as follows:
  al  in %rdi           (64 bits)
  aip in %rsi           (64 bits)
  a2  in %edx           (32 bits)
  a2p in %rcx           (64 bits)
  a3  in %8w            (16 bits)
  a3p in %r9            (64 bits)
  a4  at %rsp+8         ( 8 bits)
  a4p at %rsp+i6        (64 bits)
1  proc:
2     movq       16(%rsp)，%rax      Fetch a4p  (64 bits)
3     addq       %rdi，(%rsi)        *a1p+=a1   (64 bits)
3     addl       %edx, (%rcX)        *a2p+=a2   (32 bits)
4     addw       %r8w, (%r9)         *a3p+=a3   (16 bits)
5     movl       8(%rsp)，%edX       Fetch a4   ( 8 bits)
6     addb       %dl，(%raX)         *a4p+=a4   ( 8 bits)
7     r2t                            Return
    b)生成的汇编代码

3.7.4 栈上的局部存储
    到目前为止我们看到的大多数过程示例都不需要超出寄存器大小的本地存储区域。不过有些时候，局部数据必须存放在内存中，常见的情况包括:
    .寄存器不足够存放所有的本地数据;
    .对一个局部变量使用地址运算符‘&’，因此必须能够为它产生一个地址;
    .某些局部变量是数组或结构，因此必须能够通过数组或结构引用被访问到;
    来看一个处理地址运算符的例子，图3-31a中给出的两个函数。函数swap_add交换指针xp和yp指向的两个值，并返回这两个值的和。函数caller创建到局部变量argl和arg2的
指针，把它们传递给swap_add。图3-31b展示了caller是如何用栈帧来实现这些局部变量的。caller的代码开始的时候把栈指针减掉了16;实际上这就是在栈上分配了16个字节。S表
示栈指针的值，可以看到这段代码计算&arg2为S+8(第5行)，而&argl为S。因此可以推断局部变量argl和arg2存放在栈帧中相对于栈指针偏移量为0和8的地方。当对swap_add的调
用完成后，caller的代码会从栈上取出这两个值(第8-9行)，计算它们的差，再乘以swap_add在寄存器%rax中返回的值(第10行)。最后，该函数把栈指针加16，释放栈帧(第11行)。
通过这个例子可以看到，运行时栈提供了一种简单的、在需要时分配、函数完成时释放局部存储的机制。
    如图3-32所示，函数call_proc是一个更复杂的例子，说明x86-64栈行为的一些特性。尽管这个例子有点儿长，但还是值得仔细研究。它给出了一个必须在栈上分配局部变量存
储空间的函数，同时还要向有8个参数的函数proc传递值(图3-29)。该函数创建一个栈帧，如图3-33所示。
long swap_add(long *xp，long *yp)
{
    long x=*xp;
    long y=*yp}
    *xp=y;
    *yp=x;
    return x+Y:
}
long caller()
{
    long argl=534;
    long arg2=1057;
    long sum=swap_add(&argl,&arg2)
    long diff=argl-arg2;
    return sum*diff;
}
a) swap_ add和调用函数的代码
    long caller()
1   caller:
2     subq    $16,%rsp         Allocate 16 bytes for stack frame
3     movq    $534,(%rsp)      Store 534 in argl
4     movq    $1057,8(%rsp)    Store 1057 in arg2
5     leaq    8(%rsp),%rsi     Compute &arg2 as second argument
6     movq    %rsp,%rdi        Compute &argl as first argument
7     call    swap_add         Call swap_add(&argl,&arg2)
8     movq    (%rsp),%rdx      Cet argl
9     subq    8(%rsp),%rdx     Compute diff = argl- arg2
10    imulq   %rdx,%rax        Compute sim*diff
11    addq    $16,%rsp         Deallocate stack frame 
12    ret                      Return                                                                                                                ，..，..，..
    b)调用函数生成的汇编代码
  图3-31过程定义和调用的示例。由于会使用地址运算符，所以调用代码必须分配一个栈帧

long call-proc()
{
    long x1=1;int x2=2;
    short x3=3;char x4=4;
    proc(xl，&x1，x2，&x2，x3，&x3，x4, &x4)
    return (xi+x2)*(x3-x4);
}
    a)swap add和调用函数的代码
  图3-32调用在图3-29中定义的函数proc的代码示例。该代码创建了一个栈帧
   
    long call-proc()
1    call_proc:
       Set up arguments to proc
2      subq    $32,%rsp           Allocate 32-byte stack frame
3      movq    $1,24(%rsp)        Store i in &x1
4      movl    $2,20(%rsp)        Store 2 in &x2
5      movw    $3,18(%rsp)        Store 3 in &x3
6      movb    $4,17(%rsp)        Store 4 in &x4
7      leaq    17(%rsp).%rax      Create &x4
8      movq    %rax,8(%rsp)       Store &x4 as argument 8
9      movl    $4,(%rsp)          Store 4 as argument 7
10     leaq    18(%rsp),%r9       Pass &x3  as argument 6
11     movl    $3,%r8d            Pass 3 as argument 5
12     leaq    20(%rsp),%rcx      Pass &x2 as argument 4
13     movl    $2,%edx            Pass 2 as areument 3
14     leaq    24(%rsp),%rsi      Pass &x1 as argument 2
15     movl    $1,%edi            Pass 1 as argument 1
       Call proc
16     call    proc
       Retrieve changes to memory
17     movslq  20(%rsp),%rdx      Get x2 and convert to long
18     addq    24(%rsp),%rdx      Compute xl+x2
19     movswl  18(%rsp)，%eax     Get x3 and convert to int
20     movsbl  17(%rsp)，%ecx     Get x4 and convert to int
21     subl    %ecx,%eax          Compute x3-x4
22     cltq                       Convert to long
23     imulq   %rdx,%rax          Compute (x1+x2)*(x3-x4)
24     addq    $32,%rsp           Deallocate stack frame
25     ret                        Return
25
  b)调用函数生成的汇编代码
      图3-32(续)

3.7.5 寄存器中的局部存储空间
    寄存器组是唯一被所有过程共享的资源。虽然在给定时刻只有一个过程是活动的，我们仍然必须确保当一个过程(调用者)调用另一个过程(被调用者)时，被调用者不会覆盖调
用者稍后会使用的寄存器值。为此，x86-64采用了一组统一的寄存器使用惯例，所有的过程(包括程序库)都必须遵循。
    根据惯例，寄存器%rbx,%rbp和%r12~0r15被划分为被调用者保存寄存器。当过程P调用过程Q时，Q必须保存这些寄存器的值，保证它们的值在Q返回到P时与Q被调用时是一样
的。过程Q保存一个寄存器的值不变，要么就是根本不去改变它，要么就是把原始值压人栈中，改变寄存器的值，然后在返回前从栈中弹出旧值。压入寄存器的值会在栈帧中创建标
号为“保存的寄存器”的一部分。有了这条惯例，P的代码就能安全地把值存在被调用者保存寄存器中(当然，要先把之前的值保存到栈上)，调用Q，然后继续使用寄存器中的值，不用
担心值被破坏。
    来看一个例子，图3-34a中的函数P。它两次调用Q。在第一次调用中，必须保存x的值以备后面使用。类似地，在第二次调用中，也必须保存Q(y)的值。图3-34b中，可以看
到GCC生成的代码使用了两个被调用者保存寄存器:%rbp保存x和%rbx保存计算出来的Q(y)的值。在函数的开头，把这两个寄存器的值保存到栈中(第2-3行)。在第一次调用Q之前，
把参数x复制到%rbp(第5行)。在第二次调用Q之前，把这次调用的结果复制到%rbx(第8行)。在函数的结尾，(第13-14行)，把它们从栈中弹出，恢复这两个被调用者保存寄存器的
值。注意它们的弹出顺序与压人顺序相反，说明了栈的后进先出规则。
long P(long x, long y)
{
    long u=v(y);
    long v=q(x);
    return u+v;
}
a)调用函数
        long P(long x, long y)
        x in %rdi，y in %rsi
1    P:
2      pushq      %rbp           Save %rbp
3      pushq      %rbx           Save %rbx
4      subq       $8,%rsp        Align stack frame
5      movq       %rdi,%rbp      Save x
6      movq       %rsi,%rdi      Move y to first argument
7      call       Q              Call  Q(y)
8      movq       %rax,%rbx      Save result
9      movq       %rbp,%rdi      Move x to first argument
10     call       Q              Call  Q(x)
11     addq       %rbx,%rax      Add saved Q(y) to Q(x)
12     addq       $8,%rsp        Deallocate last part of stack
13     popq       %rbx           Restore %rbx
14     popq       %rbp           Restore %rbp
15     ret
    b)调用函数生成的汇编代码
  图3-34展示被调用者保存寄存器使用的代码。在第一次调用中，
        必须保存x的值，第二次调用中，必须保存Q(Y)的值

3.7.6 递归过程
未完待续


