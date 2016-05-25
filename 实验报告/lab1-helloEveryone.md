
#lab1-report（黄予 2013011363 计34）
##【练习1】练习1：理解通过make生成执行文件的过程
###【1.1】操作系统镜像文件ucore.img是如何一步一步生成的？(需要比较详细地解释Makefile中每一条相关命令和命令参数的含义，以及说明命令导致的结果)
makefile基础：

```
subst & patsubst：字符串替换，参数：原字符串，新字符串，文本
wildcard：文件名展开函数，例如$(wildcard *.c)将返回结果是一个所有以 '.c' 结尾的文件的列表。
suffix & prefix： 取后缀/前缀
addsuffix & addprefix：加后缀/前缀
filter：以模式过滤单词字符串，返回符合模式的单词组成的字符串
.PHONY：指明伪目标
$$:访问shell命令内定义的变量
```

本makefile相关函数：

```
totarget:给字符串加上前缀“bin/”
packetname:给字符串加上“__objs_”前缀
...
```


生成ucore.img的代码如下，可见其依赖于kernel和bootblock

```
UCOREIMG	:= $(call totarget,ucore.img)
$(UCOREIMG): $(kernel) $(bootblock)
	$(V)dd if=/dev/zero of=$@ count=10000
	$(V)dd if=$(bootblock) of=$@ conv=notrunc
	$(V)dd if=$(kernel) of=$@ seek=1 conv=notrunc
$(call create_target,ucore.img)
```
现在来看一看生成kernel的相关代码:

```
kernel = $(call totarget,kernel)
$(kernel): tools/kernel.ld
$(kernel): $(KOBJS)
	@echo + ld $@
	$(V)$(LD) $(LDFLAGS) -T tools/kernel.ld -o $@ $(KOBJS)
	@$(OBJDUMP) -S $@ > $(call asmfile,kernel)
	@$(OBJDUMP) -t $@ | $(SED) '1,/SYMBOL TABLE/d; s/ .* / /; /^$$/d' > $(call symfile,kernel)
$(call create_target,kernel)
```
可见kernel依赖于kernel.ld与$(KOBJS)，再由执行make “V=”时输出的信息：

```
ld -m    elf_i386 -nostdlib -T tools/kernel.ld -o bin/kernel  obj/kern/init/init.o obj/kern/libs/stdio.o obj/kern/libs/readline.o obj/kern/debug/panic.o obj/kern/debug/kdebug.o obj/kern/debug/kmonitor.o obj/kern/driver/clock.o obj/kern/driver/console.o obj/kern/driver/picirq.o obj/kern/driver/intr.o obj/kern/trap/trap.o obj/kern/trap/vectors.o obj/kern/trap/trapentry.o obj/kern/mm/pmm.o  obj/libs/string.o obj/libs/printfmt.o
+ cc boot/bootasm.S
```
知道$(KOBJS)指init.o、stdio.o、readline.o、panic.o、kdebug.o、kmonitor.o、clock.o等一系列位于kern文件夹下.c文件对应的.o文件，各参数的意义如下：

```
-m elf_i386：使用elf_i386仿真
-nostdlib：只使用命令行指定的库
-T tools/kernel.ld：读取链接脚本kernel.ld
```
那么init.o等文件是如何生成的呢，make “V=”时输出的信息如下：

```
+ cc kern/init/init.c
gcc -Ikern/init/ -fno-builtin -Wall -ggdb -m32 -gstabs -nostdinc  -fno-stack-protector -Ilibs/ -Ikern/debug/ -Ikern/driver/ -Ikern/trap/ -Ikern/mm/ -c kern/init/init.c -o obj/kern/init/init.o
+ cc kern/libs/stdio.c
gcc -Ikern/libs/ -fno-builtin -Wall -ggdb -m32 -gstabs -nostdinc  -fno-stack-protector -Ilibs/ -Ikern/debug/ -Ikern/driver/ -Ikern/trap/ -Ikern/mm/ -c kern/libs/stdio.c -o obj/kern/libs/stdio.o
...
```
以init.o的生成为例，其各个参数的意义为：

```
-I<dir>：增加搜索头文件的路径
-fno-builtin：除非用__builtin_前缀，否则不进行builtin函数的优化
-Wall：生成所有警告信息
-ggdb：生成debug信息以供gdb使用，这样才可用gdb调试
-m32：生成32位的代码。因本实验的qemu模拟的硬件环境为32位的80386
-gstabs：产生stabs格式的调试信息
-nostdinc：不搜索头文件的标准系统目录，否则将使用linux系统提供的服务。
-fno-stack-protector：不生成用于检测缓冲区溢出的代码。
```
现在kernel模块已经分析完毕，接下来分析第二个模块bootblock的生成，makefile相关代码为：

```
bootfiles = $(call listf_cc,boot)
$(foreach f,$(bootfiles),$(call cc_compile,$(f),$(CC),$(CFLAGS) -Os -nostdinc))
bootblock = $(call totarget,bootblock)
$(bootblock): $(call toobj,$(bootfiles)) | $(call totarget,sign)
	@echo + ld $@
	$(V)$(LD) $(LDFLAGS) -N -e start -Ttext 0x7C00 $^ -o $(call toobj,bootblock)
	@$(OBJDUMP) -S $(call objfile,bootblock) > $(call asmfile,bootblock)
	@$(OBJCOPY) -S -O binary $(call objfile,bootblock) $(call outfile,bootblock)
	@$(call totarget,sign) $(call outfile,bootblock) $(bootblock)
$(call create_target,bootblock)
```
执行make "V="的相关输出为：

```
ld -m    elf_i386 -nostdlib -N -e start -Ttext 0x7C00 obj/boot/bootasm.o obj/boot/bootmain.o -o obj/bootblock.o
```
可见bootblock.o依赖于bootasm.o和bootmain.o，其各个参数的意义为：

```
-N：不将数据对齐至页边界
-e start：设定起始地址为start
-Ttext 0x7C00：设置代码段的起始地址为7C00
```
生成bootasm.o、bootmain.o的相关输出为：

```
gcc -Iboot/ -fno-builtin -Wall -ggdb -m32 -gstabs -nostdinc  -fno-stack-protector -Ilibs/ -Os -nostdinc -c boot/bootasm.S -o obj/boot/bootasm.o
+ cc boot/bootmain.c
gcc -Iboot/ -fno-builtin -Wall -ggdb -m32 -gstabs -nostdinc  -fno-stack-protector -Ilibs/ -Os -nostdinc -c boot/bootmain.c -o obj/boot/bootmain.o
+ cc tools/sign.c
```
bootasm.o依赖于bootasm.S，bootmain.o依赖于bootmain.c，各个参数的意义为：

```
-Os：代码优化，但不增加代码的大小
```

###【1.2】一个被系统认为是符合规范的硬盘主引导扇区的特征是什么？
由sign.c代码：

```
char buf[512];
...
buf[510] = 0x55;
buf[511] = 0xAA;
```
可见主引导扇区大小为512字节，其中第511个字节为0x55，第512个字节为0xAA

##【练习2】
###【2.1】.从CPU加电后执行的第一条指令开始，单步跟踪BIOS的执行。
修改 lab1/tools/gdbinit,内容为:

```
set architecture i8086
target remote :1234
```
执行make debug即可

###【2.2】.在初始化位置0x7c00设置实地址断点,测试断点正常。
将tools/gdbinit文件内容改为：

```
file bin/kernel
set architecture i8086
target remote :1234
b *0x7c00
continue
x /16i $pc
```
运行make clean与make debug后输出为：

```
Breakpoint 1, 0x00007c00 in ?? ()
=> 0x7c00:      cli
   0x7c01:      cld
   0x7c02:      xor    %ax,%ax
   0x7c04:      mov    %ax,%ds
   0x7c06:      mov    %ax,%es
   0x7c08:      mov    %ax,%ss
   0x7c0a:      in     $0x64,%al
   0x7c0c:      test   $0x2,%al
   0x7c0e:      jne    0x7c0a
   0x7c10:      mov    $0xd1,%al
   0x7c12:      out    %al,$0x64
   0x7c14:      in     $0x64,%al
   0x7c16:      test   $0x2,%al
   0x7c18:      jne    0x7c14
   0x7c1a:      mov    $0xdf,%al
   0x7c1c:      out    %al,$0x60
```

###【2.3】从0x7c00开始跟踪代码运行,将单步跟踪反汇编得到的代码与bootasm.S和 bootblock.asm进行比较。

bootasm.S中的代码为：

```
	cli                                             # Disable interrupts
    cld                                             # String operations increment
    xorw %ax, %ax                                   # Segment number zero
    movw %ax, %ds                                   # -> Data Segment
    movw %ax, %es                                   # -> Extra Segment
    movw %ax, %ss                                   # -> Stack Segment
    inb $0x64, %al                                  # Wait for not busy(8042 input buffer empty).
    testb $0x2, %al
    jnz seta20.1
    movb $0xd1, %al                                 # 0xd1 -> port 0x64
    outb %al, $0x64 
```
bootblock.asm代码为(去掉机器码)：

```
	cli                                             # Disable interrupts
    cld                                             # String operations increment
    xorw %ax, %ax                                   # Segment number zero
    movw %ax, %ds                                   # -> Data Segment
    movw %ax, %es                                   # -> Extra Segment
    movw %ax, %ss                                   # -> Stack Segment
	nb $0x64, %al                                  # Wait for not busy(8042 input buffer empty).
    testb $0x2, %al
    jnz seta20.1
    movb $0xd1, %al                                 # 0xd1 -> port 0x64
    outb %al, $0x64                                 # 0xd1 means: write data to 8042's P2 port
    ...
```
与【2.2】对照，均相同。

###【2.4】自己找一个bootloader或内核中的代码位置，设置断点并进行测试。
将tools/gdbinit的文件内容改为：

```
file obj/bootblock.o
target remote: 1234
b readseg
continue
```
运行make debug后输出为：

```
Breakpoint 1 at 0x7c72: file boot/bootmain.c, line 68.
Breakpoint 1, readseg (va=65536, count=4096, offset=0) at boot/bootmain.c:68
(gdb) x /8i $pc
=> 0x7c72 <readseg>:    push   %ebp
   0x7c73 <readseg+1>:  mov    %esp,%ebp
   0x7c75 <readseg+3>:  push   %edi
   0x7c76 <readseg+4>:  lea    (%eax,%edx,1),%edi
   0x7c79 <readseg+7>:  mov    %ecx,%edx
   0x7c7b <readseg+9>:  shr    $0x9,%ecx
   0x7c7e <readseg+12>: push   %esi
   0x7c7f <readseg+13>: and    $0x1ff,%edx
   (gdb) info r
eax            0x10000  65536
ecx            0x0      0
edx            0x1000   4096
ebx            0x0      0
esp            0x7bec   0x7bec
ebp            0x7bf8   0x7bf8
esi            0x0      0
edi            0x0      0
eip            0x7c72   0x7c72 <readseg>
eflags         0x46     [ PF ZF ]
cs             0x8      8
ss             0x10     16
ds             0x10     16
es             0x10     16
fs             0x10     16
gs             0x10     16
```
断点设为readseg函数。

##【练习3】分析bootloader进入保护模式的过程。
###BIOS将通过读取硬盘主引导扇区到内存，并转跳到对应内存中的位置执行bootloader。请分析bootloader是如何完成从实模式进入保护模式的。
###【3.1】为何开启A20，以及如何开启A20
###【3.2】如何初始化GDT表
###【3.3】如何使能和进入保护模式
初始化环境，将寄存器清零:

```
	cli                                             #禁止中断
    cld										 	     #设置传送字串时地址指针为增
    xorw %ax, %ax                                   #将ax清零
    movw %ax, %ds                                   #将ds(data segment)清零
    movw %ax, %es                                   #将es(extra segment)清零
    movw %ax, %ss 							 	     #将ss(stack segment)清零
```
开启A20，以使用大于1M的地址空间:

```
seta20.1:
    inb $0x64, %al                                  #等待0x64端口（P1，输入端口）不忙
    testb $0x2, %al
    jnz seta20.1
    
    movb $0xd1, %al                                 #发送写命令到0x64端口（p1，输入端口）
    outb %al, $0x64     
    
seta20.2:
    inb $0x64, %al                                  #等待0x64端口不忙
    testb $0x2, %al
    jnz seta20.2

    movb $0xdf, %al                                 #将0x60端口（p2，输出端口）的第二位（A20）置1，
    outb %al, $0x60                                 #从而开启A20
```
初始化GDT表，并开启保护模式：

```
    lgdt gdtdesc								    #装载GDT全局描述符表
    movl %cr0, %eax							   	    #将cr0的PE（protect enable）位置1，开启保护模式
    orl $CR0_PE_ON, %eax
    movl %eax, %cr0
```
其中，gdtdesc的内容如下：

```
.p2align 2                                          # force 4 byte alignment
gdt:
    SEG_NULLASM                                     # null seg
    SEG_ASM(STA_X|STA_R, 0x0, 0xffffffff)           # code seg for bootloader and kernel
    SEG_ASM(STA_W, 0x0, 0xffffffff)                 # data seg for bootloader and kernel

gdtdesc:
    .word 0x17                                      # sizeof(gdt) - 1
    .long gdt                                       # address gdt
```
进入32位模式，执行之后的指令：

```
ljmp $PROT_MODE_CSEG, $protcseg
```
设置保护模式下的段寄存器，并跳转至bootmain函数：

```
.code32                                             
protcseg:
    movw $PROT_MODE_DSEG, %ax                       # 设置ds(data)、es(extra)、fs、gs、ss(stack)
    movw %ax, %ds                                   
    movw %ax, %es                                 
    movw %ax, %fs                                  
    movw %ax, %gs                                   
    movw %ax, %ss                                   

    movl $0x0, %ebp								      # 设置栈底指针为0x0
    movl $start, %esp							      # 设置栈顶指针为0x7C00
    call bootmain								      # 跳转至bootmain函数
```
综上：bootasm.S的工作为：开启A20，初始化GDT，从实模式进入保护模式。
##【练习4】分析bootloader加载ELF格式的OS的过程。
###通过阅读bootmain.c，了解bootloader如何加载ELF文件。通过分析源代码和通过qemu来运行并调试bootloader&OS，
###【4.1】bootloader如何读取硬盘扇区的？
###【4.2】bootloader是如何加载ELF格式的OS？

bootmain.c中仅有4个函数：

```
void waitdisk(void);
void readsect(void *dst, uint32_t secno);
void readseg(uintptr_t va, uint32_t count, uint32_t offset);
void bootmain(void);
```
void waitdisk(void)函数为等待硬盘不忙才返回，具体实现为：

```
static void
waitdisk(void) {
    while ((inb(0x1F7) & 0xC0) != 0x40)//端口0X1F7的高两位为01代表硬盘准备好
        /* do nothing */;
}
```

再看看void readsect(void *dst, uint32_t secno)函数：

```
/* 读取一个扇区secno到dst地址处 */
static void
readsect(void *dst, uint32_t secno) {
	//等待磁盘准备好
    waitdisk();
	
	// 指明要读写的扇区数目为1
    outb(0x1F2, 1);
    
    //指定要读取的硬盘地址以及模式
    outb(0x1F3, secno & 0xFF);				
    outb(0x1F4, (secno >> 8) & 0xFF);
    outb(0x1F5, (secno >> 16) & 0xFF);
    outb(0x1F6, ((secno >> 24) & 0xF) | 0xE0);
    
    //发送硬盘读命令
    outb(0x1F7, 0x20);                     

    //等待磁盘准备好
    waitdisk();

    /*
    由函数insl定义，该语句的含义为：
    	1.将edx设为01F0，将edi设为dst，将ecx设为（SECTSIZE / 4），即128
    	2.cld设置地址递增
    	3.repne;insl; 重复insl直到ecx = 0；insl指令：从edx拷贝4字节至edi，edi += 4；
    	4.本函数为从硬盘拷贝一个扇区至dst处
    */
    insl(0x1F0, dst, SECTSIZE / 4);
}
```

函数void readseg(uintptr_t va, uint32_t count, uint32_t offset):

```
//从硬盘的offset处读取count字节到va地址处
static void
readseg(uintptr_t va, uint32_t count, uint32_t offset) {
    uintptr_t end_va = va + count;

    va -= offset % SECTSIZE;
	
	//获得起始扇区号，第一个扇区存储bootloader的代码（bootasm.S bootmain.c）,kernel存于第二个扇区，故+1
    uint32_t secno = (offset / SECTSIZE) + 1;
	
	//读取硬盘数据
    for (; va < end_va; va += SECTSIZE, secno ++) {
        readsect((void *)va, secno);
    }
}
```
void bootmain(void)：

```
void
bootmain(void) {
    //从1号扇区开始，读取SECTSIZE * 8个字节到ELFHDR处，即读取elf头
    readseg((uintptr_t)ELFHDR, SECTSIZE * 8, 0);

    // 判断是否为合法的elf文件
    if (ELFHDR->e_magic != ELF_MAGIC) {
        goto bad;
    }
	
	//program header
    struct proghdr *ph, *eph;

    ph = (struct proghdr *)((uintptr_t)ELFHDR + ELFHDR->e_phoff);//程序头指针数组的开始
    eph = ph + ELFHDR->e_phnum;//程序头指针数组的结束
    //从硬盘读取kernel的各个段至内存
    for (; ph < eph; ph ++) {
        readseg(ph->p_va & 0xFFFFFF, ph->p_memsz, ph->p_offset);
    }

    //开始执行kernel
    ((void (*)(void))(ELFHDR->e_entry & 0xFFFFFF))();

bad:
    outw(0x8A00, 0x8A00);
    outw(0x8A00, 0x8E00);

    /* do nothing */
    while (1);
}
```
##【练习5】
kdebug.c中函数print_stackframe的实现如下：

```
void
print_stackframe(void) {
    /* LAB1 YOUR CODE : STEP 1 */
     /* (1) call read_ebp() to get the value of ebp. the type is (uint32_t);
      * (2) call read_eip() to get the value of eip. the type is (uint32_t);
      * (3) from 0 .. STACKFRAME_DEPTH
      *    (3.1) printf value of ebp, eip
      *    (3.2) (uint32_t)calling arguments [0..4] = the contents in address (unit32_t)ebp +2 [0..4]
      *    (3.3) cprintf("\n");
      *    (3.4) call print_debuginfo(eip-1) to print the C calling function name and line number, etc.
      *    (3.5) popup a calling stackframe
      *           NOTICE: the calling funciton's return addr eip  = ss:[ebp+4]
      *                   the calling funciton's ebp = ss:[ebp]
      */
    uint32_t ebp, eip;
    ebp = read_ebp();
    eip = read_eip();
    for (int i = 0; ebp != 0 && i < STACKFRAME_DEPTH; ++i){
        cprintf("ebp:0x%08x eip:0x%08x args:", ebp, eip);
        uint32_t *args = (uint32_t *)ebp + 2;
        for (int j = 0; j < 4; ++j)
            cprintf("0x%08x ", args[j]);
        cprintf("\n");
        print_debuginfo(eip - 1);
        eip = *(uint32_t *)(ebp + 4);
        ebp = *(uint32_t *)ebp;
    }
}
```
执行make qemu后得到的输出如下：

```
...
ebp:0x00007b38 eip:0x00100a27 args:0x00010094 0x00010094 0x00007b68 0x0010007f 
    kern/debug/kdebug.c:308: print_stackframe+21
ebp:0x00007b48 eip:0x00100d27 args:0x00000000 0x00000000 0x00000000 0x00007bb8 
    kern/debug/kmonitor.c:125: mon_backtrace+10
ebp:0x00007b68 eip:0x0010007f args:0x00000000 0x00007b90 0xffff0000 0x00007b94 
    kern/init/init.c:48: grade_backtrace2+19
ebp:0x00007b88 eip:0x001000a1 args:0x00000000 0xffff0000 0x00007bb4 0x00000029 
    kern/init/init.c:53: grade_backtrace1+27
ebp:0x00007ba8 eip:0x001000be args:0x00000000 0x00100000 0xffff0000 0x00100043 
    kern/init/init.c:58: grade_backtrace0+19
ebp:0x00007bc8 eip:0x001000df args:0x00000000 0x00000000 0x00000000 0x00103280 
    kern/init/init.c:63: grade_backtrace+26
ebp:0x00007be8 eip:0x00100050 args:0x00000000 0x00000000 0x00000000 0x00007c4f 
    kern/init/init.c:28: kern_init+79
ebp:0x00007bf8 eip:0x00007d6e args:0xc031fcfa 0xc08ed88e 0x64e4d08e 0xfa7502a8 
    <unknow>: -- 0x00007d6d --
...
```
解释最后一行的含义：

```
ebp:0x00007bf8 eip:0x00007d6e args:0xc031fcfa 0xc08ed88e 0x64e4d08e 0xfa7502a8 
    <unknow>: -- 0x00007d6d --
```
由bootasm.S中的代码段：

```
    movl $0x0, %ebp
    movl $start, %esp
    call bootmain
```
start的地址在0x7c00，故esp从0x7c00开始使用，call bootmain将bootmain的地址压栈，故0x7bfc处是call bootmain下一条语句的地址，0x7bf8处为old_ebp,即0x0，因此到这里回溯就终止，也因此在bootmain函数中，ebp指向0x7bf8。

由gdb的调试obj/bootblock.o的结果：

```
│107         ((void (*)(void))(ELFHDR->e_entry & 0xFFFFFF))(); 
(gdb) x /8i $pc
=> 0x7d62 <bootmain+85>:        mov    0x10018,%eax
   0x7d67 <bootmain+90>:        and    $0xffffff,%eax
   0x7d6c <bootmain+95>:        call   *%eax
   0x7d6e <bootmain+97>:        mov    $0xffff8a00,%ed
```
在bootmain.c中，进入kernel后的返回地址为0x7d6e，故在bootmain中，eip的值为0x7d6e

##【练习6】完善中断初始化和处理
###[6.1]中断描述符表（也可简称为保护模式下的中断向量表）中一个表项占多少字节？其中哪几位代表中断处理代码的入口？
8字节，7-6字节和1-0字节为offset，3-2字节为段选择子，用段选择子从GDT中获得段描述符，从段描述符取得基址加上offset即为中断处理代码的地址。
###【6.2】请编程完善kern/trap/trap.c中对中断向量表进行初始化的函数idt_init。在idt_init函数中，依次对所有中断入口进行初始化。使用mmu.h中的SETGATE宏，填充idt数组内容。每个中断的入口由tools/vectors.c生成，使用trap.c中声明的vectors数组即可。
将256个中断向量装载进idt，由于T_SYSCALL需要将权限设为用户态，故__vectors[T_SWITCH_TOK]的权限设置为3，其余设置为0，详见代码。


###[6.3]请编程完善trap.c中的中断处理函数trap，在对时钟中断进行处理的部分填写trap函数中处理时钟中断的部分，使操作系统每遇到100次时钟中断后，调用print_ticks子程序，向屏幕上打印一行文字”100 ticks”。
使用clock.c的全局变量ticks进行计数，每100次调用一次print_ticks()函数，详见代码。


