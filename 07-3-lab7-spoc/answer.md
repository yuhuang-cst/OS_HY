#hansen管程实现思路
1.	在signal与wait函数中删去next的相关部分
2.	在哲学家问题的实现中，将退出时的if...else...替换为up(&(mtp->mutex))，即释放管程的锁。
3.	修改文件仅涉及monitor.c与check——sync.c

#运行结果

截取hansen管程输出的一个片段：

```
No.1 philosopher_condvar is in phi_put_forks_condvar
phi_test_condvar: state_condvar[0] will eating
phi_test_condvar: signal self_cv[0] 
cond_signal begin: cvp c03a6278, cvp->count 1, cvp->owner->next_count 0
cond_signal end: cvp c03a6278, cvp->count 1, cvp->owner->next_count 0
No.1 philosopher_condvar leaves phi_put_forks_condvar
```
可见No.1哲学家在发出signal唤醒No.0哲学家后仍然继续执行，输出了"No.1 philosopher_condvar leaves phi_put_forks_condvar"


截取hoare管程输出的一个片段：

```
No.1 philosopher_condvar is in phi_put_forks_condvar
phi_test_condvar: state_condvar[0] will eating
phi_test_condvar: signal self_cv[0] 
cond_signal begin: cvp c03a7688, cvp->count 1, cvp->owner->next_count 0
cond_wait end:  cvp c03a7688, cvp->count 0, cvp->owner->next_count 1
No.0 philosopher_condvar leaves phi_take_forks_condvar
```
No.1哲学家在发出signal唤醒No.0哲学家后进入睡眠状态，No.0哲学家立即开始执行。

#完整运行结果

##hansen管程:

```
++ setup timer interrupts
kernel_execve: pid = 2, name = "matrix".
fork ok.
I am No.0 philosopher_condvar
Iter 1, No.0 philosopher_condvar is thinking
I am No.1 philosopher_condvar
Iter 1, No.1 philosopher_condvar is thinking
I am No.2 philosopher_condvar
Iter 1, No.2 philosopher_condvar is thinking
I am No.3 philosopher_condvar
Iter 1, No.3 philosopher_condvar is thinking
I am No.4 philosopher_condvar
Iter 1, No.4 philosopher_condvar is thinking
pid 8 is running (1000 times)!.
pid 8 done!.
pid 9 is running (1000 times)!.
pid 10 is running (1100 times)!.
pid 10 done!.
pid 11 is running (1900 times)!.
pid 12 is running (4600 times)!.
pid 13 is running (11000 times)!.
pid 14 is running (20600 times)!.
pid 15 is running (37100 times)!.
pid 16 is running (2600 times)!.
pid 17 is running (13100 times)!.
pid 18 is running (37100 times)!.
pid 19 is running (4600 times)!.
pid 20 is running (23500 times)!.
pid 21 is running (2600 times)!.
pid 22 is running (23500 times)!.
pid 23 is running (4600 times)!.
pid 24 is running (33400 times)!.
pid 25 is running (13100 times)!.
pid 26 is running (2600 times)!.
pid 27 is running (26600 times)!.
pid 28 is running (13100 times)!.
No.0 philosopher_condvar is in phi_take_forks_condvar
phi_test_condvar: state_condvar[0] will eating
phi_test_condvar: signal self_cv[0] 
cond_signal begin: cvp c03a6278, cvp->count 0, cvp->owner->next_count 0
cond_signal end: cvp c03a6278, cvp->count 0, cvp->owner->next_count 0
No.0 philosopher_condvar leaves phi_take_forks_condvar
Iter 1, No.0 philosopher_condvar is eating
No.1 philosopher_condvar is in phi_take_forks_condvar
phi_take_forks_condvar: 1 didn't get fork and will wait
cond_wait begin:  cvp c03a628c, cvp->count 0, cvp->owner->next_count 0
No.2 philosopher_condvar is in phi_take_forks_condvar
phi_test_condvar: state_condvar[2] will eating
phi_test_condvar: signal self_cv[2] 
cond_signal begin: cvp c03a62a0, cvp->count 0, cvp->owner->next_count 0
cond_signal end: cvp c03a62a0, cvp->count 0, cvp->owner->next_count 0
No.2 philosopher_condvar leaves phi_take_forks_condvar
Iter 1, No.2 philosopher_condvar is eating
No.3 philosopher_condvar is in phi_take_forks_condvar
phi_take_forks_condvar: 3 didn't get fork and will wait
cond_wait begin:  cvp c03a62b4, cvp->count 0, cvp->owner->next_count 0
No.4 philosopher_condvar is in phi_take_forks_condvar
phi_take_forks_condvar: 4 didn't get fork and will wait
cond_wait begin:  cvp c03a62c8, cvp->count 0, cvp->owner->next_count 0
pid 9 done!.
pid 11 done!.
pid 26 done!.
No.0 philosopher_condvar is in phi_put_forks_condvar
phi_test_condvar: state_condvar[4] will eating
phi_test_condvar: signal self_cv[4] 
cond_signal begin: cvp c03a62c8, cvp->count 1, cvp->owner->next_count 0
cond_signal end: cvp c03a62c8, cvp->count 1, cvp->owner->next_count 0
No.0 philosopher_condvar leaves phi_put_forks_condvar
Iter 2, No.0 philosopher_condvar is thinking
No.2 philosopher_condvar is in phi_put_forks_condvar
phi_test_condvar: state_condvar[1] will eating
phi_test_condvar: signal self_cv[1] 
cond_signal begin: cvp c03a628c, cvp->count 1, cvp->owner->next_count 0
cond_signal end: cvp c03a628c, cvp->count 1, cvp->owner->next_count 0
No.2 philosopher_condvar leaves phi_put_forks_condvar
Iter 2, No.2 philosopher_condvar is thinking
cond_wait end:  cvp c03a62c8, cvp->count 0, cvp->owner->next_count 0
No.4 philosopher_condvar leaves phi_take_forks_condvar
Iter 1, No.4 philosopher_condvar is eating
cond_wait end:  cvp c03a628c, cvp->count 0, cvp->owner->next_count 0
No.1 philosopher_condvar leaves phi_take_forks_condvar
Iter 1, No.1 philosopher_condvar is eating
pid 16 done!.
pid 21 done!.
No.0 philosopher_condvar is in phi_take_forks_condvar
phi_take_forks_condvar: 0 didn't get fork and will wait
cond_wait begin:  cvp c03a6278, cvp->count 0, cvp->owner->next_count 0
No.2 philosopher_condvar is in phi_take_forks_condvar
phi_take_forks_condvar: 2 didn't get fork and will wait
cond_wait begin:  cvp c03a62a0, cvp->count 0, cvp->owner->next_count 0
No.4 philosopher_condvar is in phi_put_forks_condvar
phi_test_condvar: state_condvar[3] will eating
phi_test_condvar: signal self_cv[3] 
cond_signal begin: cvp c03a62b4, cvp->count 1, cvp->owner->next_count 0
cond_signal end: cvp c03a62b4, cvp->count 1, cvp->owner->next_count 0
No.4 philosopher_condvar leaves phi_put_forks_condvar
Iter 2, No.4 philosopher_condvar is thinking
cond_wait end:  cvp c03a62b4, cvp->count 0, cvp->owner->next_count 0
No.3 philosopher_condvar leaves phi_take_forks_condvar
Iter 1, No.3 philosopher_condvar is eating
No.1 philosopher_condvar is in phi_put_forks_condvar
phi_test_condvar: state_condvar[0] will eating
phi_test_condvar: signal self_cv[0] 
cond_signal begin: cvp c03a6278, cvp->count 1, cvp->owner->next_count 0
cond_signal end: cvp c03a6278, cvp->count 1, cvp->owner->next_count 0
No.1 philosopher_condvar leaves phi_put_forks_condvar
Iter 2, No.1 philosopher_condvar is thinking
No.3 philosopher_condvar is in phi_put_forks_condvar
phi_test_condvar: state_condvar[2] will eating
phi_test_condvar: signal self_cv[2] 
cond_signal begin: cvp c03a62a0, cvp->count 1, cvp->owner->next_count 0
cond_signal end: cvp c03a62a0, cvp->count 1, cvp->owner->next_count 0
No.3 philosopher_condvar leaves phi_put_forks_condvar
Iter 2, No.3 philosopher_condvar is thinking
pid 19 done!.
pid 23 done!.
cond_wait end:  cvp c03a6278, cvp->count 0, cvp->owner->next_count 0
No.0 philosopher_condvar leaves phi_take_forks_condvar
Iter 2, No.0 philosopher_condvar is eating
No.4 philosopher_condvar is in phi_take_forks_condvar
phi_take_forks_condvar: 4 didn't get fork and will wait
cond_wait begin:  cvp c03a62c8, cvp->count 0, cvp->owner->next_count 0
No.1 philosopher_condvar is in phi_take_forks_condvar
phi_take_forks_condvar: 1 didn't get fork and will wait
cond_wait begin:  cvp c03a628c, cvp->count 0, cvp->owner->next_count 0
cond_wait end:  cvp c03a62a0, cvp->count 0, cvp->owner->next_count 0
No.2 philosopher_condvar leaves phi_take_forks_condvar
Iter 2, No.2 philosopher_condvar is eating
No.3 philosopher_condvar is in phi_take_forks_condvar
phi_take_forks_condvar: 3 didn't get fork and will wait
cond_wait begin:  cvp c03a62b4, cvp->count 0, cvp->owner->next_count 0
pid 12 done!.
No.0 philosopher_condvar is in phi_put_forks_condvar
phi_test_condvar: state_condvar[4] will eating
phi_test_condvar: signal self_cv[4] 
cond_signal begin: cvp c03a62c8, cvp->count 1, cvp->owner->next_count 0
cond_signal end: cvp c03a62c8, cvp->count 1, cvp->owner->next_count 0
No.0 philosopher_condvar leaves phi_put_forks_condvar
Iter 3, No.0 philosopher_condvar is thinking
No.2 philosopher_condvar is in phi_put_forks_condvar
phi_test_condvar: state_condvar[1] will eating
phi_test_condvar: signal self_cv[1] 
cond_signal begin: cvp c03a628c, cvp->count 1, cvp->owner->next_count 0
cond_signal end: cvp c03a628c, cvp->count 1, cvp->owner->next_count 0
No.2 philosopher_condvar leaves phi_put_forks_condvar
Iter 3, No.2 philosopher_condvar is thinking
cond_wait end:  cvp c03a62c8, cvp->count 0, cvp->owner->next_count 0
No.4 philosopher_condvar leaves phi_take_forks_condvar
Iter 2, No.4 philosopher_condvar is eating
cond_wait end:  cvp c03a628c, cvp->count 0, cvp->owner->next_count 0
No.1 philosopher_condvar leaves phi_take_forks_condvar
Iter 2, No.1 philosopher_condvar is eating
No.0 philosopher_condvar is in phi_take_forks_condvar
phi_take_forks_condvar: 0 didn't get fork and will wait
cond_wait begin:  cvp c03a6278, cvp->count 0, cvp->owner->next_count 0
No.2 philosopher_condvar is in phi_take_forks_condvar
phi_take_forks_condvar: 2 didn't get fork and will wait
cond_wait begin:  cvp c03a62a0, cvp->count 0, cvp->owner->next_count 0
No.4 philosopher_condvar is in phi_put_forks_condvar
phi_test_condvar: state_condvar[3] will eating
phi_test_condvar: signal self_cv[3] 
cond_signal begin: cvp c03a62b4, cvp->count 1, cvp->owner->next_count 0
cond_signal end: cvp c03a62b4, cvp->count 1, cvp->owner->next_count 0
No.4 philosopher_condvar leaves phi_put_forks_condvar
Iter 3, No.4 philosopher_condvar is thinking
cond_wait end:  cvp c03a62b4, cvp->count 0, cvp->owner->next_count 0
No.3 philosopher_condvar leaves phi_take_forks_condvar
Iter 2, No.3 philosopher_condvar is eating
No.1 philosopher_condvar is in phi_put_forks_condvar
phi_test_condvar: state_condvar[0] will eating
phi_test_condvar: signal self_cv[0] 
cond_signal begin: cvp c03a6278, cvp->count 1, cvp->owner->next_count 0
cond_signal end: cvp c03a6278, cvp->count 1, cvp->owner->next_count 0
No.1 philosopher_condvar leaves phi_put_forks_condvar
Iter 3, No.1 philosopher_condvar is thinking
pid 13 done!.
No.3 philosopher_condvar is in phi_put_forks_condvar
phi_test_condvar: state_condvar[2] will eating
phi_test_condvar: signal self_cv[2] 
cond_signal begin: cvp c03a62a0, cvp->count 1, cvp->owner->next_count 0
cond_signal end: cvp c03a62a0, cvp->count 1, cvp->owner->next_count 0
No.3 philosopher_condvar leaves phi_put_forks_condvar
Iter 3, No.3 philosopher_condvar is thinking
cond_wait end:  cvp c03a6278, cvp->count 0, cvp->owner->next_count 0
No.0 philosopher_condvar leaves phi_take_forks_condvar
Iter 3, No.0 philosopher_condvar is eating
No.4 philosopher_condvar is in phi_take_forks_condvar
phi_take_forks_condvar: 4 didn't get fork and will wait
cond_wait begin:  cvp c03a62c8, cvp->count 0, cvp->owner->next_count 0
No.1 philosopher_condvar is in phi_take_forks_condvar
phi_take_forks_condvar: 1 didn't get fork and will wait
cond_wait begin:  cvp c03a628c, cvp->count 0, cvp->owner->next_count 0
cond_wait end:  cvp c03a62a0, cvp->count 0, cvp->owner->next_count 0
No.2 philosopher_condvar leaves phi_take_forks_condvar
Iter 3, No.2 philosopher_condvar is eating
No.3 philosopher_condvar is in phi_take_forks_condvar
phi_take_forks_condvar: 3 didn't get fork and will wait
cond_wait begin:  cvp c03a62b4, cvp->count 0, cvp->owner->next_count 0
No.0 philosopher_condvar is in phi_put_forks_condvar
phi_test_condvar: state_condvar[4] will eating
phi_test_condvar: signal self_cv[4] 
cond_signal begin: cvp c03a62c8, cvp->count 1, cvp->owner->next_count 0
cond_signal end: cvp c03a62c8, cvp->count 1, cvp->owner->next_count 0
No.0 philosopher_condvar leaves phi_put_forks_condvar
Iter 4, No.0 philosopher_condvar is thinking
No.2 philosopher_condvar is in phi_put_forks_condvar
phi_test_condvar: state_condvar[1] will eating
phi_test_condvar: signal self_cv[1] 
cond_signal begin: cvp c03a628c, cvp->count 1, cvp->owner->next_count 0
cond_signal end: cvp c03a628c, cvp->count 1, cvp->owner->next_count 0
No.2 philosopher_condvar leaves phi_put_forks_condvar
Iter 4, No.2 philosopher_condvar is thinking
cond_wait end:  cvp c03a62c8, cvp->count 0, cvp->owner->next_count 0
No.4 philosopher_condvar leaves phi_take_forks_condvar
Iter 3, No.4 philosopher_condvar is eating
cond_wait end:  cvp c03a628c, cvp->count 0, cvp->owner->next_count 0
No.1 philosopher_condvar leaves phi_take_forks_condvar
Iter 3, No.1 philosopher_condvar is eating
pid 17 done!.
pid 25 done!.
pid 28 done!.
No.0 philosopher_condvar is in phi_take_forks_condvar
phi_take_forks_condvar: 0 didn't get fork and will wait
cond_wait begin:  cvp c03a6278, cvp->count 0, cvp->owner->next_count 0
No.2 philosopher_condvar is in phi_take_forks_condvar
phi_take_forks_condvar: 2 didn't get fork and will wait
cond_wait begin:  cvp c03a62a0, cvp->count 0, cvp->owner->next_count 0
No.4 philosopher_condvar is in phi_put_forks_condvar
phi_test_condvar: state_condvar[3] will eating
phi_test_condvar: signal self_cv[3] 
cond_signal begin: cvp c03a62b4, cvp->count 1, cvp->owner->next_count 0
cond_signal end: cvp c03a62b4, cvp->count 1, cvp->owner->next_count 0
No.4 philosopher_condvar leaves phi_put_forks_condvar
Iter 4, No.4 philosopher_condvar is thinking
cond_wait end:  cvp c03a62b4, cvp->count 0, cvp->owner->next_count 0
No.3 philosopher_condvar leaves phi_take_forks_condvar
Iter 3, No.3 philosopher_condvar is eating
No.1 philosopher_condvar is in phi_put_forks_condvar
phi_test_condvar: state_condvar[0] will eating
phi_test_condvar: signal self_cv[0] 
cond_signal begin: cvp c03a6278, cvp->count 1, cvp->owner->next_count 0
cond_signal end: cvp c03a6278, cvp->count 1, cvp->owner->next_count 0
No.1 philosopher_condvar leaves phi_put_forks_condvar
Iter 4, No.1 philosopher_condvar is thinking
No.3 philosopher_condvar is in phi_put_forks_condvar
phi_test_condvar: state_condvar[2] will eating
phi_test_condvar: signal self_cv[2] 
cond_signal begin: cvp c03a62a0, cvp->count 1, cvp->owner->next_count 0
cond_signal end: cvp c03a62a0, cvp->count 1, cvp->owner->next_count 0
No.3 philosopher_condvar leaves phi_put_forks_condvar
Iter 4, No.3 philosopher_condvar is thinking
cond_wait end:  cvp c03a6278, cvp->count 0, cvp->owner->next_count 0
No.0 philosopher_condvar leaves phi_take_forks_condvar
Iter 4, No.0 philosopher_condvar is eating
No.4 philosopher_condvar is in phi_take_forks_condvar
phi_take_forks_condvar: 4 didn't get fork and will wait
cond_wait begin:  cvp c03a62c8, cvp->count 0, cvp->owner->next_count 0
No.1 philosopher_condvar is in phi_take_forks_condvar
phi_take_forks_condvar: 1 didn't get fork and will wait
cond_wait begin:  cvp c03a628c, cvp->count 0, cvp->owner->next_count 0
cond_wait end:  cvp c03a62a0, cvp->count 0, cvp->owner->next_count 0
No.2 philosopher_condvar leaves phi_take_forks_condvar
Iter 4, No.2 philosopher_condvar is eating
No.3 philosopher_condvar is in phi_take_forks_condvar
phi_take_forks_condvar: 3 didn't get fork and will wait
cond_wait begin:  cvp c03a62b4, cvp->count 0, cvp->owner->next_count 0
No.0 philosopher_condvar is in phi_put_forks_condvar
phi_test_condvar: state_condvar[4] will eating
phi_test_condvar: signal self_cv[4] 
cond_signal begin: cvp c03a62c8, cvp->count 1, cvp->owner->next_count 0
cond_signal end: cvp c03a62c8, cvp->count 1, cvp->owner->next_count 0
No.0 philosopher_condvar leaves phi_put_forks_condvar
No.0 philosopher_condvar quit
No.2 philosopher_condvar is in phi_put_forks_condvar
phi_test_condvar: state_condvar[1] will eating
phi_test_condvar: signal self_cv[1] 
cond_signal begin: cvp c03a628c, cvp->count 1, cvp->owner->next_count 0
cond_signal end: cvp c03a628c, cvp->count 1, cvp->owner->next_count 0
No.2 philosopher_condvar leaves phi_put_forks_condvar
No.2 philosopher_condvar quit
cond_wait end:  cvp c03a62c8, cvp->count 0, cvp->owner->next_count 0
No.4 philosopher_condvar leaves phi_take_forks_condvar
Iter 4, No.4 philosopher_condvar is eating
cond_wait end:  cvp c03a628c, cvp->count 0, cvp->owner->next_count 0
No.1 philosopher_condvar leaves phi_take_forks_condvar
Iter 4, No.1 philosopher_condvar is eating
No.4 philosopher_condvar is in phi_put_forks_condvar
phi_test_condvar: state_condvar[3] will eating
phi_test_condvar: signal self_cv[3] 
cond_signal begin: cvp c03a62b4, cvp->count 1, cvp->owner->next_count 0
cond_signal end: cvp c03a62b4, cvp->count 1, cvp->owner->next_count 0
No.4 philosopher_condvar leaves phi_put_forks_condvar
No.4 philosopher_condvar quit
cond_wait end:  cvp c03a62b4, cvp->count 0, cvp->owner->next_count 0
No.3 philosopher_condvar leaves phi_take_forks_condvar
Iter 4, No.3 philosopher_condvar is eating
No.1 philosopher_condvar is in phi_put_forks_condvar
No.1 philosopher_condvar leaves phi_put_forks_condvar
No.1 philosopher_condvar quit
pid 14 done!.
No.3 philosopher_condvar is in phi_put_forks_condvar
No.3 philosopher_condvar leaves phi_put_forks_condvar
No.3 philosopher_condvar quit
pid 22 done!.
pid 20 done!.
pid 27 done!.
pid 18 done!.
pid 24 done!.
pid 15 done!.
matrix pass.
all user-mode processes have quit.
init check memory pass.
kernel panic at kern/process/proc.c:493:
    initproc exit.

Welcome to the kernel debug monitor!!
Type 'help' for a list of commands.
```

##hoare管程

```
++ setup timer interrupts
kernel_execve: pid = 2, name = "matrix".
fork ok.
I am No.0 philosopher_sema
Iter 1, No.0 philosopher_sema is thinking
I am No.1 philosopher_sema
Iter 1, No.1 philosopher_sema is thinking
I am No.2 philosopher_sema
Iter 1, No.2 philosopher_sema is thinking
I am No.3 philosopher_sema
Iter 1, No.3 philosopher_sema is thinking
I am No.4 philosopher_sema
Iter 1, No.4 philosopher_sema is thinking
I am No.0 philosopher_condvar
Iter 1, No.0 philosopher_condvar is thinking
I am No.1 philosopher_condvar
Iter 1, No.1 philosopher_condvar is thinking
I am No.2 philosopher_condvar
Iter 1, No.2 philosopher_condvar is thinking
I am No.3 philosopher_condvar
Iter 1, No.3 philosopher_condvar is thinking
I am No.4 philosopher_condvar
Iter 1, No.4 philosopher_condvar is thinking
pid 13 is running (1000 times)!.
pid 13 done!.
pid 14 is running (1000 times)!.
pid 14 done!.
pid 15 is running (1100 times)!.
pid 16 is running (1900 times)!.
pid 17 is running (4600 times)!.
pid 18 is running (11000 times)!.
pid 19 is running (20600 times)!.
pid 20 is running (37100 times)!.
pid 21 is running (2600 times)!.
pid 22 is running (13100 times)!.
pid 23 is running (37100 times)!.
pid 24 is running (4600 times)!.
pid 25 is running (23500 times)!.
pid 26 is running (2600 times)!.
pid 27 is running (23500 times)!.
pid 28 is running (4600 times)!.
pid 29 is running (33400 times)!.
pid 30 is running (13100 times)!.
pid 31 is running (2600 times)!.
pid 32 is running (26600 times)!.
pid 33 is running (13100 times)!.
Iter 1, No.0 philosopher_sema is eating
Iter 1, No.2 philosopher_sema is eating
No.0 philosopher_condvar is in phi_take_forks_condvar
phi_test_condvar: state_condvar[0] will eating
phi_test_condvar: signal self_cv[0] 
cond_signal begin: cvp c03a7688, cvp->count 0, cvp->owner->next_count 0
cond_signal end: cvp c03a7688, cvp->count 0, cvp->owner->next_count 0
No.0 philosopher_condvar leaves phi_take_forks_condvar
Iter 1, No.0 philosopher_condvar is eating
No.1 philosopher_condvar is in phi_take_forks_condvar
phi_take_forks_condvar: 1 didn't get fork and will wait
cond_wait begin:  cvp c03a769c, cvp->count 0, cvp->owner->next_count 0
No.2 philosopher_condvar is in phi_take_forks_condvar
phi_test_condvar: state_condvar[2] will eating
phi_test_condvar: signal self_cv[2] 
cond_signal begin: cvp c03a76b0, cvp->count 0, cvp->owner->next_count 0
cond_signal end: cvp c03a76b0, cvp->count 0, cvp->owner->next_count 0
No.2 philosopher_condvar leaves phi_take_forks_condvar
Iter 1, No.2 philosopher_condvar is eating
No.3 philosopher_condvar is in phi_take_forks_condvar
phi_take_forks_condvar: 3 didn't get fork and will wait
cond_wait begin:  cvp c03a76c4, cvp->count 0, cvp->owner->next_count 0
No.4 philosopher_condvar is in phi_take_forks_condvar
phi_take_forks_condvar: 4 didn't get fork and will wait
cond_wait begin:  cvp c03a76d8, cvp->count 0, cvp->owner->next_count 0
pid 15 done!.
pid 16 done!.
pid 21 done!.
pid 31 done!.
Iter 2, No.0 philosopher_sema is thinking
Iter 2, No.2 philosopher_sema is thinking
No.0 philosopher_condvar is in phi_put_forks_condvar
phi_test_condvar: state_condvar[4] will eating
phi_test_condvar: signal self_cv[4] 
cond_signal begin: cvp c03a76d8, cvp->count 1, cvp->owner->next_count 0
Iter 1, No.4 philosopher_sema is eating
Iter 1, No.1 philosopher_sema is eating
cond_wait end:  cvp c03a76d8, cvp->count 0, cvp->owner->next_count 1
No.4 philosopher_condvar leaves phi_take_forks_condvar
Iter 1, No.4 philosopher_condvar is eating
pid 26 done!.
cond_signal end: cvp c03a76d8, cvp->count 0, cvp->owner->next_count 0
No.0 philosopher_condvar leaves phi_put_forks_condvar
Iter 2, No.0 philosopher_condvar is thinking
Iter 2, No.4 philosopher_sema is thinking
Iter 1, No.3 philosopher_sema is eating
Iter 2, No.1 philosopher_sema is thinking
No.2 philosopher_condvar is in phi_put_forks_condvar
phi_test_condvar: state_condvar[1] will eating
phi_test_condvar: signal self_cv[1] 
cond_signal begin: cvp c03a769c, cvp->count 1, cvp->owner->next_count 0
Iter 2, No.3 philosopher_sema is thinking
pid 24 done!.
pid 28 done!.
Iter 2, No.0 philosopher_sema is eating
Iter 2, No.2 philosopher_sema is eating
cond_wait end:  cvp c03a769c, cvp->count 0, cvp->owner->next_count 1
No.1 philosopher_condvar leaves phi_take_forks_condvar
Iter 1, No.1 philosopher_condvar is eating
cond_signal end: cvp c03a769c, cvp->count 0, cvp->owner->next_count 0
No.2 philosopher_condvar leaves phi_put_forks_condvar
Iter 2, No.2 philosopher_condvar is thinking
No.4 philosopher_condvar is in phi_put_forks_condvar
phi_test_condvar: state_condvar[3] will eating
phi_test_condvar: signal self_cv[3] 
cond_signal begin: cvp c03a76c4, cvp->count 1, cvp->owner->next_count 0
cond_wait end:  cvp c03a76c4, cvp->count 0, cvp->owner->next_count 1
No.3 philosopher_condvar leaves phi_take_forks_condvar
Iter 1, No.3 philosopher_condvar is eating
pid 17 done!.
cond_signal end: cvp c03a76c4, cvp->count 0, cvp->owner->next_count 0
No.4 philosopher_condvar leaves phi_put_forks_condvar
Iter 2, No.4 philosopher_condvar is thinking
Iter 3, No.0 philosopher_sema is thinking
Iter 3, No.2 philosopher_sema is thinking
No.0 philosopher_condvar is in phi_take_forks_condvar
phi_take_forks_condvar: 0 didn't get fork and will wait
cond_wait begin:  cvp c03a7688, cvp->count 0, cvp->owner->next_count 0
Iter 2, No.4 philosopher_sema is eating
Iter 2, No.1 philosopher_sema is eating
No.2 philosopher_condvar is in phi_take_forks_condvar
phi_take_forks_condvar: 2 didn't get fork and will wait
cond_wait begin:  cvp c03a76b0, cvp->count 0, cvp->owner->next_count 0
Iter 3, No.4 philosopher_sema is thinking
Iter 2, No.3 philosopher_sema is eating
Iter 3, No.1 philosopher_sema is thinking
No.1 philosopher_condvar is in phi_put_forks_condvar
phi_test_condvar: state_condvar[0] will eating
phi_test_condvar: signal self_cv[0] 
cond_signal begin: cvp c03a7688, cvp->count 1, cvp->owner->next_count 0
cond_wait end:  cvp c03a7688, cvp->count 0, cvp->owner->next_count 1
No.0 philosopher_condvar leaves phi_take_forks_condvar
Iter 2, No.0 philosopher_condvar is eating
pid 18 done!.
Iter 3, No.3 philosopher_sema is thinking
Iter 3, No.0 philosopher_sema is eating
Iter 3, No.2 philosopher_sema is eating
cond_signal end: cvp c03a7688, cvp->count 0, cvp->owner->next_count 0
No.1 philosopher_condvar leaves phi_put_forks_condvar
Iter 2, No.1 philosopher_condvar is thinking
Iter 4, No.0 philosopher_sema is thinking
Iter 4, No.2 philosopher_sema is thinking
Iter 3, No.4 philosopher_sema is eating
Iter 3, No.1 philosopher_sema is eating
No.3 philosopher_condvar is in phi_put_forks_condvar
phi_test_condvar: state_condvar[2] will eating
phi_test_condvar: signal self_cv[2] 
cond_signal begin: cvp c03a76b0, cvp->count 1, cvp->owner->next_count 0
pid 22 done!.
pid 30 done!.
pid 33 done!.
Iter 4, No.4 philosopher_sema is thinking
Iter 3, No.3 philosopher_sema is eating
Iter 4, No.1 philosopher_sema is thinking
cond_wait end:  cvp c03a76b0, cvp->count 0, cvp->owner->next_count 1
No.2 philosopher_condvar leaves phi_take_forks_condvar
Iter 2, No.2 philosopher_condvar is eating
cond_signal end: cvp c03a76b0, cvp->count 0, cvp->owner->next_count 0
No.3 philosopher_condvar leaves phi_put_forks_condvar
Iter 2, No.3 philosopher_condvar is thinking
No.4 philosopher_condvar is in phi_take_forks_condvar
phi_take_forks_condvar: 4 didn't get fork and will wait
cond_wait begin:  cvp c03a76d8, cvp->count 0, cvp->owner->next_count 0
Iter 4, No.3 philosopher_sema is thinking
Iter 4, No.0 philosopher_sema is eating
Iter 4, No.2 philosopher_sema is eating
No.0 philosopher_condvar is in phi_put_forks_condvar
phi_test_condvar: state_condvar[4] will eating
phi_test_condvar: signal self_cv[4] 
cond_signal begin: cvp c03a76d8, cvp->count 1, cvp->owner->next_count 0
cond_wait end:  cvp c03a76d8, cvp->count 0, cvp->owner->next_count 1
No.4 philosopher_condvar leaves phi_take_forks_condvar
Iter 2, No.4 philosopher_condvar is eating
No.0 philosopher_sema quit
No.2 philosopher_sema quit
Iter 4, No.4 philosopher_sema is eating
Iter 4, No.1 philosopher_sema is eating
cond_signal end: cvp c03a76d8, cvp->count 0, cvp->owner->next_count 0
No.0 philosopher_condvar leaves phi_put_forks_condvar
Iter 3, No.0 philosopher_condvar is thinking
No.1 philosopher_condvar is in phi_take_forks_condvar
phi_take_forks_condvar: 1 didn't get fork and will wait
cond_wait begin:  cvp c03a769c, cvp->count 0, cvp->owner->next_count 0
No.2 philosopher_condvar is in phi_put_forks_condvar
phi_test_condvar: state_condvar[1] will eating
phi_test_condvar: signal self_cv[1] 
cond_signal begin: cvp c03a769c, cvp->count 1, cvp->owner->next_count 0
No.4 philosopher_sema quit
Iter 4, No.3 philosopher_sema is eating
No.1 philosopher_sema quit
cond_wait end:  cvp c03a769c, cvp->count 0, cvp->owner->next_count 1
No.1 philosopher_condvar leaves phi_take_forks_condvar
Iter 2, No.1 philosopher_condvar is eating
cond_signal end: cvp c03a769c, cvp->count 0, cvp->owner->next_count 0
No.2 philosopher_condvar leaves phi_put_forks_condvar
Iter 3, No.2 philosopher_condvar is thinking
pid 19 done!.
No.3 philosopher_sema quit
No.3 philosopher_condvar is in phi_take_forks_condvar
phi_take_forks_condvar: 3 didn't get fork and will wait
cond_wait begin:  cvp c03a76c4, cvp->count 0, cvp->owner->next_count 0
No.4 philosopher_condvar is in phi_put_forks_condvar
phi_test_condvar: state_condvar[3] will eating
phi_test_condvar: signal self_cv[3] 
cond_signal begin: cvp c03a76c4, cvp->count 1, cvp->owner->next_count 0
cond_wait end:  cvp c03a76c4, cvp->count 0, cvp->owner->next_count 1
No.3 philosopher_condvar leaves phi_take_forks_condvar
Iter 2, No.3 philosopher_condvar is eating
cond_signal end: cvp c03a76c4, cvp->count 0, cvp->owner->next_count 0
No.4 philosopher_condvar leaves phi_put_forks_condvar
Iter 3, No.4 philosopher_condvar is thinking
No.0 philosopher_condvar is in phi_take_forks_condvar
phi_take_forks_condvar: 0 didn't get fork and will wait
cond_wait begin:  cvp c03a7688, cvp->count 0, cvp->owner->next_count 0
pid 27 done!.
No.2 philosopher_condvar is in phi_take_forks_condvar
phi_take_forks_condvar: 2 didn't get fork and will wait
cond_wait begin:  cvp c03a76b0, cvp->count 0, cvp->owner->next_count 0
No.1 philosopher_condvar is in phi_put_forks_condvar
phi_test_condvar: state_condvar[0] will eating
phi_test_condvar: signal self_cv[0] 
cond_signal begin: cvp c03a7688, cvp->count 1, cvp->owner->next_count 0
cond_wait end:  cvp c03a7688, cvp->count 0, cvp->owner->next_count 1
No.0 philosopher_condvar leaves phi_take_forks_condvar
Iter 3, No.0 philosopher_condvar is eating
pid 25 done!.
cond_signal end: cvp c03a7688, cvp->count 0, cvp->owner->next_count 0
No.1 philosopher_condvar leaves phi_put_forks_condvar
Iter 3, No.1 philosopher_condvar is thinking
No.3 philosopher_condvar is in phi_put_forks_condvar
phi_test_condvar: state_condvar[2] will eating
phi_test_condvar: signal self_cv[2] 
cond_signal begin: cvp c03a76b0, cvp->count 1, cvp->owner->next_count 0
pid 32 done!.
cond_wait end:  cvp c03a76b0, cvp->count 0, cvp->owner->next_count 1
No.2 philosopher_condvar leaves phi_take_forks_condvar
Iter 3, No.2 philosopher_condvar is eating
cond_signal end: cvp c03a76b0, cvp->count 0, cvp->owner->next_count 0
No.3 philosopher_condvar leaves phi_put_forks_condvar
Iter 3, No.3 philosopher_condvar is thinking
No.4 philosopher_condvar is in phi_take_forks_condvar
phi_take_forks_condvar: 4 didn't get fork and will wait
cond_wait begin:  cvp c03a76d8, cvp->count 0, cvp->owner->next_count 0
No.0 philosopher_condvar is in phi_put_forks_condvar
phi_test_condvar: state_condvar[4] will eating
phi_test_condvar: signal self_cv[4] 
cond_signal begin: cvp c03a76d8, cvp->count 1, cvp->owner->next_count 0
cond_wait end:  cvp c03a76d8, cvp->count 0, cvp->owner->next_count 1
No.4 philosopher_condvar leaves phi_take_forks_condvar
Iter 3, No.4 philosopher_condvar is eating
cond_signal end: cvp c03a76d8, cvp->count 0, cvp->owner->next_count 0
No.0 philosopher_condvar leaves phi_put_forks_condvar
Iter 4, No.0 philosopher_condvar is thinking
No.1 philosopher_condvar is in phi_take_forks_condvar
phi_take_forks_condvar: 1 didn't get fork and will wait
cond_wait begin:  cvp c03a769c, cvp->count 0, cvp->owner->next_count 0
No.2 philosopher_condvar is in phi_put_forks_condvar
phi_test_condvar: state_condvar[1] will eating
phi_test_condvar: signal self_cv[1] 
cond_signal begin: cvp c03a769c, cvp->count 1, cvp->owner->next_count 0
cond_wait end:  cvp c03a769c, cvp->count 0, cvp->owner->next_count 1
No.1 philosopher_condvar leaves phi_take_forks_condvar
Iter 3, No.1 philosopher_condvar is eating
cond_signal end: cvp c03a769c, cvp->count 0, cvp->owner->next_count 0
No.2 philosopher_condvar leaves phi_put_forks_condvar
Iter 4, No.2 philosopher_condvar is thinking
No.3 philosopher_condvar is in phi_take_forks_condvar
phi_take_forks_condvar: 3 didn't get fork and will wait
cond_wait begin:  cvp c03a76c4, cvp->count 0, cvp->owner->next_count 0
pid 29 done!.
No.4 philosopher_condvar is in phi_put_forks_condvar
phi_test_condvar: state_condvar[3] will eating
phi_test_condvar: signal self_cv[3] 
cond_signal begin: cvp c03a76c4, cvp->count 1, cvp->owner->next_count 0
cond_wait end:  cvp c03a76c4, cvp->count 0, cvp->owner->next_count 1
No.3 philosopher_condvar leaves phi_take_forks_condvar
Iter 3, No.3 philosopher_condvar is eating
pid 23 done!.
cond_signal end: cvp c03a76c4, cvp->count 0, cvp->owner->next_count 0
No.4 philosopher_condvar leaves phi_put_forks_condvar
Iter 4, No.4 philosopher_condvar is thinking
No.0 philosopher_condvar is in phi_take_forks_condvar
phi_take_forks_condvar: 0 didn't get fork and will wait
cond_wait begin:  cvp c03a7688, cvp->count 0, cvp->owner->next_count 0
No.2 philosopher_condvar is in phi_take_forks_condvar
phi_take_forks_condvar: 2 didn't get fork and will wait
cond_wait begin:  cvp c03a76b0, cvp->count 0, cvp->owner->next_count 0
No.1 philosopher_condvar is in phi_put_forks_condvar
phi_test_condvar: state_condvar[0] will eating
phi_test_condvar: signal self_cv[0] 
cond_signal begin: cvp c03a7688, cvp->count 1, cvp->owner->next_count 0
cond_wait end:  cvp c03a7688, cvp->count 0, cvp->owner->next_count 1
No.0 philosopher_condvar leaves phi_take_forks_condvar
Iter 4, No.0 philosopher_condvar is eating
pid 20 done!.
matrix pass.
cond_signal end: cvp c03a7688, cvp->count 0, cvp->owner->next_count 0
No.1 philosopher_condvar leaves phi_put_forks_condvar
Iter 4, No.1 philosopher_condvar is thinking
No.3 philosopher_condvar is in phi_put_forks_condvar
phi_test_condvar: state_condvar[2] will eating
phi_test_condvar: signal self_cv[2] 
cond_signal begin: cvp c03a76b0, cvp->count 1, cvp->owner->next_count 0
cond_wait end:  cvp c03a76b0, cvp->count 0, cvp->owner->next_count 1
No.2 philosopher_condvar leaves phi_take_forks_condvar
Iter 4, No.2 philosopher_condvar is eating
cond_signal end: cvp c03a76b0, cvp->count 0, cvp->owner->next_count 0
No.3 philosopher_condvar leaves phi_put_forks_condvar
Iter 4, No.3 philosopher_condvar is thinking
No.4 philosopher_condvar is in phi_take_forks_condvar
phi_take_forks_condvar: 4 didn't get fork and will wait
cond_wait begin:  cvp c03a76d8, cvp->count 0, cvp->owner->next_count 0
No.0 philosopher_condvar is in phi_put_forks_condvar
phi_test_condvar: state_condvar[4] will eating
phi_test_condvar: signal self_cv[4] 
cond_signal begin: cvp c03a76d8, cvp->count 1, cvp->owner->next_count 0
cond_wait end:  cvp c03a76d8, cvp->count 0, cvp->owner->next_count 1
No.4 philosopher_condvar leaves phi_take_forks_condvar
Iter 4, No.4 philosopher_condvar is eating
cond_signal end: cvp c03a76d8, cvp->count 0, cvp->owner->next_count 0
No.0 philosopher_condvar leaves phi_put_forks_condvar
No.0 philosopher_condvar quit
No.1 philosopher_condvar is in phi_take_forks_condvar
phi_take_forks_condvar: 1 didn't get fork and will wait
cond_wait begin:  cvp c03a769c, cvp->count 0, cvp->owner->next_count 0
No.2 philosopher_condvar is in phi_put_forks_condvar
phi_test_condvar: state_condvar[1] will eating
phi_test_condvar: signal self_cv[1] 
cond_signal begin: cvp c03a769c, cvp->count 1, cvp->owner->next_count 0
cond_wait end:  cvp c03a769c, cvp->count 0, cvp->owner->next_count 1
No.1 philosopher_condvar leaves phi_take_forks_condvar
Iter 4, No.1 philosopher_condvar is eating
cond_signal end: cvp c03a769c, cvp->count 0, cvp->owner->next_count 0
No.2 philosopher_condvar leaves phi_put_forks_condvar
No.2 philosopher_condvar quit
No.3 philosopher_condvar is in phi_take_forks_condvar
phi_take_forks_condvar: 3 didn't get fork and will wait
cond_wait begin:  cvp c03a76c4, cvp->count 0, cvp->owner->next_count 0
No.4 philosopher_condvar is in phi_put_forks_condvar
phi_test_condvar: state_condvar[3] will eating
phi_test_condvar: signal self_cv[3] 
cond_signal begin: cvp c03a76c4, cvp->count 1, cvp->owner->next_count 0
cond_wait end:  cvp c03a76c4, cvp->count 0, cvp->owner->next_count 1
No.3 philosopher_condvar leaves phi_take_forks_condvar
Iter 4, No.3 philosopher_condvar is eating
cond_signal end: cvp c03a76c4, cvp->count 0, cvp->owner->next_count 0
No.4 philosopher_condvar leaves phi_put_forks_condvar
No.4 philosopher_condvar quit
No.1 philosopher_condvar is in phi_put_forks_condvar
No.1 philosopher_condvar leaves phi_put_forks_condvar
No.1 philosopher_condvar quit
No.3 philosopher_condvar is in phi_put_forks_condvar
No.3 philosopher_condvar leaves phi_put_forks_condvar
No.3 philosopher_condvar quit
all user-mode processes have quit.
init check memory pass.
kernel panic at kern/process/proc.c:493:
    initproc exit.

Welcome to the kernel debug monitor!!
Type 'help' for a list of commands.
```



