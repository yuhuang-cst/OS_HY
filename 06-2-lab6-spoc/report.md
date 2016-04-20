# 理解调度算法支撑框架的执行过程

make qemu输出为：

```
check_swap() succeeded!
++ setup timer interrupts
cpu_idle:: call schedule
schedule:: process pid = 0 name = idle enqueue
schedule:: call sched_class_pick_next
stride_pick_next:: old p->lab6_stride = 0, p->lab6_priority = 0
stride_pick_next:: new p->lab6_stride = 2147483647, p->lab6_priority = 0
schedule:: process pid = 1 name = init dequeue
do_wait:: call schedule
schedule:: call sched_class_pick_next
stride_pick_next:: old p->lab6_stride = 0, p->lab6_priority = 0
stride_pick_next:: new p->lab6_stride = 2147483647, p->lab6_priority = 0
schedule:: process pid = 2 name =  dequeue
kernel_execve: pid = 2, name = "exit".
I am the parent. Forking the child...
I am parent, fork a child pid 3
I am the parent, waiting now..
do_wait:: call schedule
schedule:: call sched_class_pick_next
stride_pick_next:: old p->lab6_stride = 0, p->lab6_priority = 0
stride_pick_next:: new p->lab6_stride = 2147483647, p->lab6_priority = 0
schedule:: process pid = 3 name =  dequeue
I am the child.
trap:: call schedule
schedule:: process pid = 3 name =  enqueue
schedule:: call sched_class_pick_next
stride_pick_next:: old p->lab6_stride = 2147483647, p->lab6_priority = 0
stride_pick_next:: new p->lab6_stride = 4294967294, p->lab6_priority = 0
schedule:: process pid = 3 name =  dequeue
trap:: call schedule
schedule:: process pid = 3 name =  enqueue
schedule:: call sched_class_pick_next
stride_pick_next:: old p->lab6_stride = 4294967294, p->lab6_priority = 0
stride_pick_next:: new p->lab6_stride = 2147483645, p->lab6_priority = 0
schedule:: process pid = 3 name =  dequeue
trap:: call schedule
schedule:: process pid = 3 name =  enqueue
schedule:: call sched_class_pick_next
stride_pick_next:: old p->lab6_stride = 2147483645, p->lab6_priority = 0
stride_pick_next:: new p->lab6_stride = 4294967292, p->lab6_priority = 0
schedule:: process pid = 3 name =  dequeue
trap:: call schedule
schedule:: process pid = 3 name =  enqueue
schedule:: call sched_class_pick_next
stride_pick_next:: old p->lab6_stride = 4294967292, p->lab6_priority = 0
stride_pick_next:: new p->lab6_stride = 2147483643, p->lab6_priority = 0
schedule:: process pid = 3 name =  dequeue
trap:: call schedule
schedule:: process pid = 3 name =  enqueue
schedule:: call sched_class_pick_next
stride_pick_next:: old p->lab6_stride = 2147483643, p->lab6_priority = 0
stride_pick_next:: new p->lab6_stride = 4294967290, p->lab6_priority = 0
schedule:: process pid = 3 name =  dequeue
trap:: call schedule
schedule:: process pid = 3 name =  enqueue
schedule:: call sched_class_pick_next
stride_pick_next:: old p->lab6_stride = 4294967290, p->lab6_priority = 0
stride_pick_next:: new p->lab6_stride = 2147483641, p->lab6_priority = 0
schedule:: process pid = 3 name =  dequeue
trap:: call schedule
schedule:: process pid = 3 name =  enqueue
schedule:: call sched_class_pick_next
stride_pick_next:: old p->lab6_stride = 2147483641, p->lab6_priority = 0
stride_pick_next:: new p->lab6_stride = 4294967288, p->lab6_priority = 0
schedule:: process pid = 3 name =  dequeue
do_exit:: call schedule
schedule:: call sched_class_pick_next
stride_pick_next:: old p->lab6_stride = 2147483647, p->lab6_priority = 0
stride_pick_next:: new p->lab6_stride = 4294967294, p->lab6_priority = 0
schedule:: process pid = 2 name = exit dequeue
waitpid 3 ok.
exit pass.
do_exit:: call schedule
schedule:: call sched_class_pick_next
stride_pick_next:: old p->lab6_stride = 2147483647, p->lab6_priority = 0
stride_pick_next:: new p->lab6_stride = 4294967294, p->lab6_priority = 0
schedule:: process pid = 1 name = init dequeue
init_main:: call schedule
schedule:: process pid = 1 name = init enqueue
schedule:: call sched_class_pick_next
stride_pick_next:: old p->lab6_stride = 4294967294, p->lab6_priority = 0
stride_pick_next:: new p->lab6_stride = 2147483645, p->lab6_priority = 0
schedule:: process pid = 1 name = init dequeue
all user-mode processes have quit.
init check memory pass.
kernel panic at kern/process/proc.c:460:
    initproc exit.

Welcome to the kernel debug monitor!!
Type 'help' for a list of commands
```

#stride溢出问题

设两个进程的stride分别为a，b,均为无符号数。

1.	当b增加了步长，但是没有溢出，那么a-b作为有符号数是小于0的，即应该选a作为next；
2.	当b增加了步长，溢出。注意到b增加的步长最长为 BIG_STRIDE = 0x7FFFFFFF。
	*	则增加步长之前：a >= b >= 0x7FFFFFFF。
	*	增加步长之后：a >= 0x7FFFFFFF > b，且注意到这时a - b作为无符号数大于0x7FFFFFFF，意味着作为有符号数小于0，即应选a作为next。
3.	实际上，BIG_STRIDE可取不超过0x7FFFFFFF的数，当取大于0x7FFFFFFF的数时会出问题，例如：
	*	若BIG_STRIDE = 0x8FFFFFFF，a = b = 0x7FFFFFFF, b += 0x8FFFFFFF, b = 0x0, a - b = 0x7FFFFFFF > 0 失败
	*	若BIG_STRIDE = 0x7FFFFFFF，a = b = 0x7FFFFFFF, b += 0x7FFFFFFF, b = 0x80000000, a - b = FFFFFFFF < 0 成功
	
	
	
	
	