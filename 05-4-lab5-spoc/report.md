#(spoc) 理解用户进程的生命周期。

程序输出以及注释如下：
```
proc_run:: current pid 0 -> 1 					//从进程0切换到进程1，名为init，执行代码为init_main
init_main begin 								//开始执行init_main
kernel_thread begin 							//init_main调用kernel_thread创建新进程，执行代码为user_main
do_fork: pid = 2, state = 0 					//kernel_thread调用do_fork，复制进程空间
wakeup_proc:: pid = 2, state 0 -> state 2, wait_state 0 -> wait_state 0 //唤醒进程2，进程2由uninitialized变为runnable态
do_wait:: pid = 1, state 2 -> state 1, wait_state 0 -> wait_state -2147483647 //进程1进入等待状态
proc_run:: current pid 1 -> 2 					//从进程1切换到进程2
proc_run:: tf->CS 0 -> 0 						//进程1、进程2的trapframe均返回内核态
user_main:: current pid: 2, KERNEL_EXECVE(hello) //开始执行进程2的user_main函数
kernel_execve:: CPL = 0 						//user_main函数调用kernel_execve函数创建用户进程，名为hello
__KERNEL_EXECVE: pid = 2, name = "hello".
kernel_execve: pid = 2, name = "hello".
kernel_execve:: CPL = 0 						//在kernel_execve函数中，进行SYS_exec的系统调用之前，处于内核态
sys_exec:: CPL = 0								//中断处理程序
do_execve:: CPL = 0								
syscall:: tf->cs CPL = 3						//中断处理函数执行完毕时，trapframe中的CPL已经被改为3，将返回用户态
Hello world!!.									//hello.c用户态程序开始执行
I am process 2. CPL = 3
hello pass.										//hello.c用户态程序执行完毕
do_exit:: CPL = 0, pid = 2, state 2 -> state 3 //返回内核态，进程2执行完毕，变为僵尸态
wakeup_proc:: pid = 1, state 1 -> state 2, wait_state -2147483647 -> wait_state 0 //唤醒进程1
proc_run:: current pid 2 -> 1 //切换进程2至进程1
all user-mode processes have quit.
init check memory pass.
kernel panic at kern/process/proc.c:458:
    initproc exit.
```