#黄予 2013011363 计34
#练习1：分配并初始化一个进程控制块
##【1.1】设计流程
实现如下：

```
static struct proc_struct *
alloc_proc(void) {
    struct proc_struct *proc = kmalloc(sizeof(struct proc_struct));
    if (proc != NULL) {
     proc->state = PROC_UNINIT;
     proc->pid = -1;
     proc->runs = 0;
     proc->kstack = 0;
     proc->need_resched = 0;
     proc->parent = NULL;
     proc->mm = NULL; //mm对内核线程没有用，故设为NULL
     memset(&(proc->context), 0, sizeof(struct context)); //将context中的寄存器信息均置为0
     proc->tf = NULL;
     proc->cr3 = boot_cr3;//保存页表的物理地址，仅有内核线程，故设为boot_cr3;若为用户进程，则应设为mm->pgdir
     proc->flags = 0;
     memset(proc->name, 0, PROC_NAME_LEN+1);

    }
    return proc;
}
```
将proc_struct的各个成员初始化一下，更具体的值在do_fork函数中会进行设置。

##【1.2】说明proc_struct中context和trapframe *tf成员变量含义和在本实验中的作用
*	context

	其结构如下：

	```
	struct context {
    	uint32_t eip;
    	uint32_t esp;
    	uint32_t ebx;
    	uint32_t ecx;
    	uint32_t edx;
    	uint32_t esi;
    	uint32_t edi;
    	uint32_t ebp;
	};
	```
	可知context存储着一个进程的通用寄存器信息。context结构在需要切换线程时用到：
	
	```
	void
	proc_run(struct proc_struct *proc) {
    	if (proc != current) {
        	bool intr_flag;
        	struct proc_struct *prev = current, *next = proc;
        	local_intr_save(intr_flag);
        	{
            	current = proc;
            	load_esp0(next->kstack + KSTACKSIZE);
            	lcr3(next->cr3);
            	switch_to(&(prev->context), &(next->context));
        	}
        	local_intr_restore(intr_flag);
    	}
	}
	```
	switch_to函数将当前的通用寄存器信息保存至prev->context中，同时将next->context中的通用寄存器信息导入通用寄存器，从而设置好下一个进程的寄存器环境。

*	trapframe

	其结构如下：
	
	```
	struct trapframe {
    	struct pushregs tf_regs;
    	uint16_t tf_gs;
    	uint16_t tf_padding0;
    	uint16_t tf_fs;
    	uint16_t tf_padding1;
    	uint16_t tf_es;
    	uint16_t tf_padding2;
    	uint16_t tf_ds;
    	uint16_t tf_padding3;
    	uint32_t tf_trapno;
    	/* below here defined by x86 hardware */
    	uint32_t tf_err;
    	uintptr_t tf_eip;
    	uint16_t tf_cs;
    	uint16_t tf_padding4;
    	uint32_t tf_eflags;
    	/* below here only when crossing rings, such as from user to kernel */
    	uintptr_t tf_esp;
    	uint16_t tf_ss;
    	uint16_t tf_padding5;
	} __attribute__((packed));
	```
	可知trapframe存储了当前运行程序的段寄存器信息和程序运行的状态等。
	
	```
.text
.globl __alltraps
__alltraps:

    pushl %ds
    pushl %es
    pushl %fs
    pushl %gs
    pushal

    movl $GD_KDATA, %eax
    movw %ax, %ds
    movw %ax, %es

    pushl %esp

    call trap
	```
	
	```
	void
	trap(struct trapframe *tf) {
    	// dispatch based on what type of trap occurred
    	trap_dispatch(tf);
	}
	```
	在发生中断时，当前的段寄存器等信息都会保存至trapframe。tf为中断帧trapframe的指针，总是指向内核栈的某个位置，中断帧记录了在中断之前进程的状态。当从内核跳回用户态的时候，需要使用trapframe中的内容恢复用户态的执行环境。


#练习2：为新创建的内核线程分配资源
##【2.1】设计流程

流程：

1.	调用alloc_proc，首先获得一块用户信息块
2.	为进程分配一个内核栈。
3.	设置新进程的父亲为原进程
4.	复制原进程的内存管理信息到新进程（但内核线程不必做此事）
5.	复制原进程上下文context与trapframe到新进程
6.	将新进程添加到进程列表
7.	唤醒新进程
8.	返回新进程的pid

代码：

```
int
do_fork(uint32_t clone_flags, uintptr_t stack, struct trapframe *tf) {
    int ret = -E_NO_FREE_PROC;
    struct proc_struct *proc;
    if (nr_process >= MAX_PROCESS) {
        goto fork_out;
    }
    ret = -E_NO_MEM;
    //调用alloc_proc，首先获得一块用户信息块
    proc = alloc_proc();
    if (proc == NULL) 
      goto fork_out;
    //为进程分配一个内核栈。
    if (setup_kstack(proc) < 0){
      goto bad_fork_cleanup_kstack;
    }
    //设置新进程的父亲为原进程
    proc->parent = current;
    //复制原进程的内存管理信息到新进程（但内核线程不必做此事）
    copy_mm(clone_flags, proc);
    //复制原进程上下文context与trapframe到新进程
    copy_thread(proc, stack, tf);
    //将新进程添加到进程列表
    proc->pid = get_pid(); //获取pid
    hash_proc(proc); //将进程控制块加入哈希表
    list_add(&proc_list, &(proc->list_link)); //将进程控制块加入双向线性链表
    ++nr_process; //进程数+1
    //唤醒新进程
    wakeup_proc(proc);
    ret = proc->pid;

fork_out:
    return ret;

bad_fork_cleanup_kstack:
    put_kstack(proc);
bad_fork_cleanup_proc:
    kfree(proc);
    goto fork_out;
}

```

##【2.2】说明ucore是否做到给每个新fork的线程一个唯一的id

do_fork函数使用get_pid函数来为新线程分配一个唯一的id，get_pid函数如下：

```
// get_pid - alloc a unique pid for process
static int
get_pid(void) {
    static_assert(MAX_PID > MAX_PROCESS);
    struct proc_struct *proc;
    list_entry_t *list = &proc_list, *le;
    static int next_safe = MAX_PID, last_pid = MAX_PID;
    if (++ last_pid >= MAX_PID) {
        last_pid = 1;
        goto inside;
    }
    if (last_pid >= next_safe) {
    inside:
        next_safe = MAX_PID;
    repeat:
        le = list;
        while ((le = list_next(le)) != list) {
            proc = le2proc(le, list_link);
            if (proc->pid == last_pid) {
                if (++ last_pid >= next_safe) {
                    if (last_pid >= MAX_PID) {
                        last_pid = 1;
                    }
                    next_safe = MAX_PID;
                    goto repeat;
                }
            }
            else if (proc->pid > last_pid && next_safe > proc->pid) {
                next_safe = proc->pid;
            }
        }
    }
    return last_pid;
}
```
可以做到唯一id，get_pid将扫描一遍现有进程，next_safe存储的是当前扫描过的进程号中大于last_pid的最小值，last_pid存储的是待返回的pid，若有进程与last_pid冲突，则last_pid自增1。若扫描完毕时last_pid并没有达到next_safe，则last_pid可用；若在扫描过程中超过了next_safe，则说明last_pid到next_safe之间的进程号都已被占用，此时则将next_safe置最大，last_pid不变，重新扫描。

#练习3：阅读代码，理解 proc_run 函数和它调用的函数如何完成进程切换的。

proc_run代码以及注释如下：

```
void
proc_run(struct proc_struct *proc) {
    if (proc != current) {
        bool intr_flag;
        struct proc_struct *prev = current, *next = proc;
        local_intr_save(intr_flag);//禁止中断使能
        {
            current = proc; //将当前的进程控制块赋值为下一个进程的控制块
            load_esp0(next->kstack + KSTACKSIZE); //设置下一个进程的内核栈
            lcr3(next->cr3); //设置下一个进程的页表基址
            switch_to(&(prev->context), &(next->context));//将当前寄存器的信息存进prev->context，将next->context中寄存器的信息装载进寄存器
        }
        local_intr_restore(intr_flag);//开启中断使能
    }
}
```

switch_to函数的说明在https://piazza.com/class/i5j09fnsl7k5x0?cid=743，说的很详细，在此不再赘述。

##【3.1】在本实验的执行过程中，创建且运行了几个内核线程？
两个内核线程，kern_init函数中调用proc_init函数，proc_init代码如下：

```
void
proc_init(void) {
    int i;

    list_init(&proc_list);
    for (i = 0; i < HASH_LIST_SIZE; i ++) {
        list_init(hash_list + i);
    }

    if ((idleproc = alloc_proc()) == NULL) {
        panic("cannot alloc idleproc.\n");
    }

    idleproc->pid = 0;
    idleproc->state = PROC_RUNNABLE;
    idleproc->kstack = (uintptr_t)bootstack;
    idleproc->need_resched = 1;
    set_proc_name(idleproc, "idle");
    nr_process ++;

    current = idleproc;

    int pid = kernel_thread(init_main, "Hello world!!", 0);
    if (pid <= 0) {
        panic("create init_main failed.\n");
    }

    initproc = find_proc(pid);
    set_proc_name(initproc, "init");

    assert(idleproc != NULL && idleproc->pid == 0);
    assert(initproc != NULL && initproc->pid == 1);
}
```
通过alloc_proc()创建了idleproc内核线程，又通过kernel_thread函数创建了initproc内核线程。idleproc为第一个运行的内核线程，之后将通过cpu_idle函数切换到initproc内核线程，打出Hello World字样。



##【3.2】语句local_intr_save(intr_flag);....local_intr_restore(intr_flag);在这里有何作用?请说明理由
local_intr_save为禁止中断，local_intr_restore为允许中断。即开始进行进程切换前禁止中断，进程切换完成后开启中断使能，这样可避免在进程切换过程中发生中断被打断，造成混乱。


