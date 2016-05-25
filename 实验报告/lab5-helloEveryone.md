#黄予 2013011363 计34
#练习1: 加载应用程序并执行
##设计实现：

1.	代码段寄存器赋值为USER_CS，即CPL = 3，index = GD_UTEXT
2.	数据段寄存器赋值为USER_DS，即CPL = 3，index = GD_UDATA
3.	esp赋值为0xB0000000，即用户栈的起始地址
4.	eip赋值为elf->e_entry，即用户程序的第一条指令
5.	eflags设为允许中断

代码如下：

```
/* LAB5:EXERCISE1 2013011363
     * should set tf_cs,tf_ds,tf_es,tf_ss,tf_esp,tf_eip,tf_eflags
     * NOTICE: If we set trapframe correctly, then the user level process can return to USER MODE from kernel. So
     *          tf_cs should be USER_CS segment (see memlayout.h)
     *          tf_ds=tf_es=tf_ss should be USER_DS segment
     *          tf_esp should be the top addr of user stack (USTACKTOP)
     *          tf_eip should be the entry point of this binary program (elf->e_entry)
     *          tf_eflags should be set to enable computer to produce Interrupt
     */
    tf->tf_cs = USER_CS;
    tf->tf_ds = tf_es = tf_ss = USER_DS;
    tf->tf_esp = USTACKTOP;
    tf_eip = elf->e_entry;
    tf->tf_eflags = FL_IF;
```

##用户程序的加载执行过程
1.	proc_init函数中,```int pid = kernel_thread(init_main, NULL, 0);```创建执行init_main的内核进程，pid为1
2.	init_main函数中，```int pid = kernel_thread(user_main, NULL, 0);```创建执行user_main的内核进程，pid为2
3.	user_main函数中，```KERNEL_EXECVE(exit);```调用kernel_execve函数创建用户进程，执行的程序为exit.c
4.	kernel_execve函数中：

	```
	static int
	kernel_execve(const char *name, unsigned char *binary, size_t size) {
    	int ret, len = strlen(name);
    	asm volatile (
        	"int %1;"
        	: "=a" (ret)
        	: "i" (T_SYSCALL), "0" (SYS_exec), "d" (name), "c" (len), "b" (binary), "D" (size)
        	: "memory");
    	return ret;
	}
	```
	通过系统调用进入中断处理例程sys_exec
5.	sys_exec函数中：

	```
	static int
	sys_exec(uint32_t arg[]) {
    	const char *name = (const char *)arg[0];
    	size_t len = (size_t)arg[1];
    	unsigned char *binary = (unsigned char *)arg[2];
    	size_t size = (size_t)arg[3];
    	return do_execve(name, len, binary, size);
	}
	```
	调用do_execve函数装载用户程序exit.c
6.	do_execve函数中调用了load_icode函数，装载ELF格式的程序
7.	load_icode函数中，设置了当前执行进程(pid = 2)的trapframe，设置trapframe的段寄存器的CPL/RPL为用户态，tf_eip为用户程序exit.c的第一条指令地址，这样，从中断服务例程使用trapframe跳转回去时，便会跳到用户exit.c的第一条指令去执行。
8.	中断服务例程处理完毕，利用trapframe中的信息返回，开始执行exit.c

#练习2: 父进程复制自己的内存空间给子进程
改动之处：trap.c时钟中断，idt_init；proc.c alloc_...

##设计实现
1.	找到进程A的虚拟地址
2.	找到进程B的虚拟地址
3.	复制页
4.	建立逻辑地址start与新分配的内存页npage的映射关系

copy_range函数中，增加如下内容：

```
void *src_kvaddr = page2kva(page); //找到进程A的虚拟地址
void *dst_kvaddr = page2kva(npage); //找到进程B的虚拟地址
memcpy(dst_kvaddr, src_kvaddr, PGSIZE); //复制进程A的页表
ret = page_insert(to, npage, start, perm); //建立npage与start的联系
```

##其他改动之处
do_fork函数中，修改之处：

```
assert(current->wait_state == 0);
/*list_add(&proc_list, &(proc->list_link)); //将进程控制块加入双向线性链表
++nr_process; //进程数+1*/
set_links(proc);
```

trap.c中的idt_init函数中，加入系统调用门：

```
void
idt_init(void) {
    extern uintptr_t __vectors[];
    for (int i = 0; i < sizeof(idt) / sizeof(struct gatedesc); ++i){
        SETGATE(idt[i], 0, GD_KTEXT, __vectors[i], DPL_KERNEL);
    }
    SETGATE(idt[T_SYSCALL], 1, GD_KTEXT, __vectors[T_SYSCALL], DPL_USER);
    lidt(&idt_pd);
}
```
trap.c中的trap_dispatch函数，将print_ticks()注释掉，同时加上current->need_resched = 1;

```
 	case IRQ_OFFSET + IRQ_TIMER:
        if(++ticks == TICK_NUM){
            //print_ticks();
            ticks = 0;
            assert(current != NULL);
            current->need_resched = 1;
        }
        break;
```
porc.c中的alloc_proc函数，增加内容：

```
proc->wait_state = 0;
proc->cptr = proc->optr = proc->yptr = NULL;
```

##COW的概要设计
1.	在copy_range函数中不进行内存拷贝
2.	当一个进程试图写两个进程共享的只读页时，将产生缺页异常，调用do_pgfault函数
3.	在do_pgfault函数中，查看虚地址对应的页表项，查看是否试图写一个只读页，且该只读页为fork出来的进程所共享，若是则新复制一个内存页，更新映射关系，对新页进行写入。


#练习3: 阅读分析源代码，理解进程执行 fork/exec/wait/exit 的实现，以及系统调用的实现

##分析fork/exec/wait/exit在实现中是如何影响进程的执行状态的
1.	do_fork函数调用alloc_proc生成一个进程控制块，此时进程控制块的state为PROC_UNINIT，wait_state为0；do_fork函数进而调用wakeup_proc函数，将state更新为PROC_RUNNABLE，wait_state为0。
2.	do_wait函数使当前进程的state由PROC_RUNNABLE态进入PROC_SLEEPING态，wait_state进入WT_CHILD态。
3.	do_execv不改变进程的执行状态，负责装载进程
4.	do_exit函数将进程state更新为PROC_ZOMBIE态，等待父进程回收资源。

##系统调用的实现
1.	通过INT 80产生系统调用，例如：

	```
	#define T_SYSCALL           0x80
	#define SYS_exec            4
	int ret, len = strlen(name);
    asm volatile (
        "int %1;"
        : "=a" (ret)
        : "i" (T_SYSCALL), "0" (SYS_exec), "d" (name), "c" (len), "b" (binary), "D" (size)
        : "memory");
    return ret;
	```
	此处系统调用号为SYS_exec
2.	进入trap函数，进而进入trap_dispatch函数：
3.	
	```
	case T_SYSCALL:
        syscall();
        break;
	```
4.	在syscall()函数中，根据系统调用号执行相应的处理函数。处理函数被组织成一个函数指针的数组：

	```
	static int (*syscalls[])(uint32_t arg[]) = {
    [SYS_exit]              sys_exit,
    [SYS_fork]              sys_fork,
    [SYS_wait]              sys_wait,
    [SYS_exec]              sys_exec,
    [SYS_yield]             sys_yield,
    [SYS_kill]              sys_kill,
    [SYS_getpid]            sys_getpid,
    [SYS_putc]              sys_putc,
    [SYS_pgdir]             sys_pgdir,
	};
	```
	此时通过系统调用号进行索引，即可调用对应的函数，例如sys_exec()函数如下：
	
	```
	static int
	sys_exec(uint32_t arg[]) {
    	const char *name = (const char *)arg[0];
    	size_t len = (size_t)arg[1];
    	unsigned char *binary = (unsigned char *)arg[2];
    	size_t size = (size_t)arg[3];
    	return do_execve(name, len, binary, size);
	}
	```
5.	执行完syscall()函数后返回INT 80的下一条语句继续执行。




##给出ucore中一个用户态进程的执行状态生命周期图
（包括执行状态，执行状态之间的变换关系，以及产生变换的事件或函数调用）

----alloc_proc()----> PROC_UNINIT ----wakeup_proc()----> PROC_RUNNABLE ----do_wait()----> PROC_SLEEPING ----wakeup_proc()----> PROC_RUNNABLE ----do_exit()----> PROC_ZOMBIE


#make grade输出：

```
hy@hy-virtual-machine:~/work/OS/ucore_os_lab_github/ucore_os_lab/labcodes/lab5$ make grade
badsegment:              (2.0s)
  -check result:                             OK
  -check output:                             OK
divzero:                 (2.0s)
  -check result:                             OK
  -check output:                             OK
softint:                 (2.0s)
  -check result:                             OK
  -check output:                             OK
faultread:               (2.1s)
  -check result:                             OK
  -check output:                             OK
faultreadkernel:         (2.1s)
  -check result:                             OK
  -check output:                             OK
hello:                   (1.9s)
  -check result:                             OK
  -check output:                             OK
testbss:                 (2.0s)
  -check result:                             OK
  -check output:                             OK
pgdir:                   (2.1s)
  -check result:                             OK
  -check output:                             OK
yield:                   (2.0s)
  -check result:                             OK
  -check output:                             OK
badarg:                  (2.0s)
  -check result:                             OK
  -check output:                             OK
exit:                    (1.9s)
  -check result:                             OK
  -check output:                             OK
spin:                    (4.9s)
  -check result:                             OK
  -check output:                             OK
waitkill:                (13.8s)
  -check result:                             OK
  -check output:                             OK
forktest:                (1.9s)
  -check result:                             OK
  -check output:                             OK
forktree:                (2.1s)
  -check result:                             OK
  -check output:                             OK
Total Score: 150/150
```




