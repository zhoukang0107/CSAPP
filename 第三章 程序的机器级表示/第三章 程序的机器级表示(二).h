3.6 控制
    机器代码提供两种基本的低级机制来实现有条件的行为:测试数据值，然后根据测试的结果来改变控制流或者数据流。

3.6.1 条件码
    除了整数寄存器，CPU还维护着一组单个位的条件码(condition code)寄存器，它们
描述了最近的算术或逻辑操作的属性。可以检测这些寄存器来执行条件分支指令。最常用
的条件码有:
CF:进位标志。最近的操作使最高位产生了进位。可用来检查无符号操作的溢出。
ZF:零标志。最近的操作得出的结果为。。
SF:符号标志。最近的操作得到的结果为负数。
OF:溢出标志。最近的操作导致一个补码溢出—正溢出或负溢出。
    比如说，假设我们用一条ADD指令完成等价于C表达式t=a+ b的功能，这里变量
a, b和t都是整型的。然后，根据下面的C表达式来设置条件码:
    CF     (unsigned) t<(unsigned) a无符号溢出
    ZF     (t==0) 零
    SF     (t<0)  负数
    OF     (a<0==b<0)&&(t<0!=a<0)有符号溢出

CMP指令和TEST指令只设置条件码而不改变任何其他寄存器;CMP指令根据两个操作数之差来设置条件码。除了只设置条件码而不更新目的寄存器之外，CMP指令与SUB指令的行为是一样的。
TEST指令的行为与AND指令一样，除了它们只设置条件码而不改变目的寄存器的值。典型的用法是，两个操作数是一样的(例如，testq %rax, orax用来检查%rax是负数、零，还是正数)，
或其中的一个操作数是一个掩码，用来指示哪些位应该被测试。

3.6.2 访问条件码
    条件码通常不会直接读取，常用的使用方法有三种:
    1)可以根据条件码的某种组合，将一个字节设置为0或者1；
    2)可以条件跳转到程序的某个其他的部分；
    3)可以有条件地传送数据。
    对于第一种情况，图3-14中描述的指令根据条件码的某种组合，将一个字节设置为0或者1。我们将这一整类指令称为SET指令;它们之间的区别就在于它们考虑的条件码的组合是什么，
这些指令名字的不同后缀指明了它们所考虑的条件码的组合。这些指令的后缀表示不同的条件而不是操作数大小。例如，指令setl和setb表示"小于时设置(set less)"和"低于时设置(set below)"，而不是"设置长字(set
long word)"和"设置字节(set byte)".
┌──────────┬────────┬─────────────────────┬──────────────────────┐
│ 指令       同义名    效果                  设置条件              │
├──────────┼────────┼─────────────────────┼──────────────────────┤
│sete    D │setz    │ D <—— ZF            │相等/零               
│setne   D │setnz   │ D <—— ~ZF           │不等/非零              
│          │        │                     │                   
│sets    D │        │ D <—— SF            │负数                   
│setns   D │        │ D <—— ~SF           │非负数                 
│          │        │                     │                      
│setg    D │setnle  │ D <—— ~(SF^OF)&^ZF  │大于(有符号>)          
│setge   D │setnl   │ D <—— ~(SF^OF)      │大于等于(有符号>=)     
│setl    D │setnge  │ D <—— SF^OF         │小于(有符号<)          
│setle   D │setng   │ D <—— (SF^OF)|ZF    │小于等于(有符号<=)    
│          │        │                     │                      
│seta    D │setnbe  │ D <—— ~CF&~ZF       │超过(无符号>)          
│setae   D │setnb   │ D <—— ~CF           │超过或相等(无符号>=)  
│setb    D │setnae  │ D <—— CF            │低于(无符号<)          
│setbe   D │setna   │ D <—— CF|ZF         │低于或相等(无符号<=)    
└──────────┴────────┴─────────────────────┴──────────────────────┘
    图3-14 SET指令.每条指令根据条件码的某种组合，将一个字节设置为0
        或者1有些指令有“同义名”，也就是同一条机器指令有别的名字

int comp(data_t a, data_t b)
a in %rdi，b in %rsi
1    comp:
2      cmpq     %rsi，%rdi      Compare a:b
3      setl     %a1             Set low-order byte of %eax to 0 or 1
4      movzbl义al，}eax         Clear rest of %eax (and rest of %rax)
5      ret
movzbl指令不仅会把%eax的高3个字节清零，还会把整个寄存器%rax的高4个字节都清零。

3.6.3 跳转指令
    跳转(jump)指令会导致执行切换到程序中一个全新的位置。在汇编代码中，这些跳转的目的地通常用一个标号(label)指明。考虑下面的汇编代码序列(完全是人为编造的):
    movq   $O,%rax           Set }rax to 0
    jmp    L1                goto  .L1
    movq   (%rax),%rdX       Null pointer dereference (skipped)
    .L1:   
    popq   %rdx              Jump target
    指令jmp .L1会导致程序跳过movq指令，而从popq指令开始继续执行。在产生目标代码文件时，汇编器会确定所有带标号指令的地址，并将跳转目标(目的指令的地址)编码为
跳转指令的一部分。
    
    图3-15列举了不同的跳转指令。j mp指令是无条件跳转。它可以是直接跳转，即跳转目标是作为指令的一部分编码的;也可以是间接跳转，即跳转目标是从寄存器或内存位置中
读出的。汇编语言中，直接跳转是给出一个标号作为跳转目标的，例如上面所示代码中的标号".L1"。间接跳转的写法是'*'后面跟一个操作数指示符。
┌──────────────┬──────┬──────────────┬──────────────────────┐
│指令            同义名 跳转条件        描述                  │
├──────────────┼──────┼──────────────┼──────────────────────┤
│jmp  Label    │      │1             │直接跳转              
│jmp  *Operand │      │1             │间接跳转              
│              │      │              │                      
│je   Label    │Jz    │ZF            │相等/零               
│jne  Label    │Jnz   │~ZF           │不相等/非零           
│              │      │SF            │                   
│js   Label    │      │SF            │负数                  
│jns  Label    │      │~SF           │非负数                
│              │      │              │                     
│jg   Label    │jnle  │~(SF^OF)&~ZF  │大于(有符号>)        
│jge  Label    │jnl   │~(SF^OF)      │大于或等于(有符号>=) 
│j1   Label    │jnge  │SF^OF         │小于(有符号<)         
│j1e  Label    │Jng   │(SF^OF)|ZF    │小于或等于(有符号<=) 
│              │      │              │小于或等于(有符号<=) 
│ja   Label    │jnbe  │~CF&~ZF       │超过(无符号>)         
│jae  Label    │jnb   │~CF           │超过或相等(无符号>=) 
│jb   Label    │Jnae  │CF            │低于(无符号<)         
│jbe  Label    │Jna   │CF|ZF         │低于或相等(无符号<=) 
└──────────────┴──────┴──────────────┴──────────────────────┘
    图3-15jump指令。当跳转条件满足时，这些指令会跳转到一条带标
    号的目的地。有些指令有“同义名”，也就是同一条机器指令的别名
    
    表中所示的其他跳转指令都是有条件的—它们根据条件码的某种组合，或者跳转，或者继续执行代码序列中下一条指令。这些指令的名字和跳转条件与SET指令的名字和设置条件
是相匹配的(参见图3-14 )。同SET指令一样，一些底层的机器指令有多个名字。条件跳转只能是直接跳转。

3.6.4 跳转指令的编码
    在汇编代码中，跳转目标用符号标号书写。汇编器，以及后来的链接器，会产生跳转目标的适当编码。跳转指令有几种不同的编码，但是最常用都是PC相对的(PC-relative)。
也就是，它们会将目标指令的地址与紧跟在跳转指令后面那条指令的地址之间的差作为编码。这些地址偏移量可以编码为1, 2或4个字节。第二种编码方法是给出“绝对”地址，用4个
字节直接指定目标。汇编器和链接器会选择适当的跳转目的编码。
    下面是一个PC相对寻址的例子，这个函数的汇编代码由编译文件branch.。产生。它
包含两个跳转:第2行的jmp指令前向跳转到更高的地址，而第7行的jg指令后向跳转
到较低的地址。

1   movq     %rdi，%rax
2   jmp      .L2
3  .L3:
4   sarq     %rax
5  .L2:
6   testq    %rax,%rax
7   jg       .L3
8   rep;ret
汇编器产生的“.o格式的反汇编版本如下:
1    0:    48 89 f8    mov     %rdi,%rax
2    3:    eb 03       jmp     8 <loop+0x8>
3    5:    48 d1 f8    sar     %rax
4    8:    48 85 c0    test    %rax,%rax
5    b:    7f   f8     jg      5 <loop+0x5>
6    d:    f3 c3       repz    retq


3.6.5 用条件控制来实现条件分支
    将条件表达式和语句从C语言翻译成机器代码，最常用的方式是结合有条件和无条件跳转。例如，图3-16a给出了一个计算两数之差绝对值的函数的C代码。这个函数有一个副作用，
会增加两个计数器，编码为全局变量it_cn七和ge_cnt之一。GCC产生的汇编代码如图3-16 c所示。把这个机器代码再转换成C语言，我们称之为函数gotodiff_se(图3-16b)。它使用
了C语言中的goto语句，这个语句类似于汇编代码中的无条件跳转。使用goto语句通常认为是一种不好的编程风格，因为它会使代码非常难以阅读和调试。本文中使用goto语句，是
为了构造描述汇编代码程序控制流的C程序。我们称这样的编程风格为"goto代码"。
long lt_cnt = 0;
long ge_cnt = 0;
long absdiff_se(long x, long y)
{
    long result;
    if (x<Y){
        lt_cnt++;
        result=Y-x;
    }
    else{
        ge_cnt++;
        result=x-Y}
    }
    return result;
}
a)原始的C语言代码

1   long gotodiff_se(long x, long y)
2   {
3       long result;
4       if  (x>=Y)
5           goto x_ge_y;
6       lt_cnt++;
7       result=y-x;
8       return result;
9     x_ge_y:
10      ge_cnt++;
11      result=x-Y:
12      return result;
13  }
b)与之等价的goto版本

    long absdiff_se(long x, long y)
    x in %rdi，y in %rsi
1   absdiff_se:
2     cmpq    %rsi，%rdi         Compare x:y
3     jge     .L2                If>=goto x_ge_y
4     addq    $1，lt_cnt(%rip)   it_cnt++
5     movq    %rsi，%rax
6     subq    %rdi，%rax         result = y-x
7     ret                        Return
8   .L2:                         x_ge_y:
9     addq    $1，ge_cnt(%rip)   ge_cnt++
10    movq    %rdi，%rax
11    subq    %rsi，%rax         result = x-y
12    ret                        Return                                                                         
c)产生的汇编代码
图3-16条件语句的编译。a)C过程absdiff_se包含一个if-else语句;b)C过程gotodiff_se模拟了汇编代码的控制;c)给出了产生的汇编代码

C语言中的if-else语句的通用形式模板如下:
    if  (test-expr)
        then-statement
    else
        else-statement

这里test-expr是一个整数表达式，它的取值为0(解释为"假")或者为非0(解释为"真")。两个分支语句中(then-statement或else-statement)只会执行一个。
    对于这种通用形式，汇编实现通常会使用下面这种形式，这里，我们用C语法来描述控制流:
    t = test-expr;
    if(!t)
      goto false;
    then-statement
    goto done;
false:
    else-statement
done:
    也就是，汇编器为then-statement和else-statement产生各自的代码块。它会插人条件和无条件分支，以保证能执行正确的代码块。

3.6.6 用条件传送来实现条件分支





























