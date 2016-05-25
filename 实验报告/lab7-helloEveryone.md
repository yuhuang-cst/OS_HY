#黄予 2013011363 计34
#练习1: 理解内核级信号量的实现和基于内核级信号量的哲学家就餐问题

##【1.1】请在实验报告中给出内核级信号量的设计描述，并说其大致执行流流程。
###信号量的设计描述与执行流程
信号量的结构如下：

```
typedef struct {
    int value;
    wait_queue_t wait_queue;
} semaphore_t;
```
value代表可用资源的数目，wait_queue代表因资源不足而进入等待的进程队列。

信号量的P操作相当于ucore中的__down()函数，代码以及相应的注释说明如下：

```
static __noinline uint32_t __down(semaphore_t *sem, uint32_t wait_state) {
    bool intr_flag;
    local_intr_save(intr_flag);

    //若value大于0，说明有资源可以使用，那么将该资源减1，直接返回
    if (sem->value > 0) {
        sem->value --;
        local_intr_restore(intr_flag);
        return 0;
    }

    //若value小于等于0，说明没有资源可以使用，那么使该进程进入睡眠状态，加入该信号量的等待队列中，选择其他进程进行执行
    wait_t __wait, *wait = &__wait;
    wait_current_set(&(sem->wait_queue), wait, wait_state);
    local_intr_restore(intr_flag);

    schedule();

    //若本进程从睡眠状态被唤醒，说明有资源可以使用，那么将该进程从等待队列中移出，__down函数返回后继续执行本进程。
    local_intr_save(intr_flag);
    wait_current_del(&(sem->wait_queue), wait);
    local_intr_restore(intr_flag);

    if (wait->wakeup_flags != wait_state) {
        return wait->wakeup_flags;
    }
    return 0;
}
```
为了防止被打断，在_down()函数中有相应的中断的开启与关闭操作。在调度其他进程之前，即schedule函数之前，会把中断打开，以免干扰到下一个进程的执行。wait的结构的设置可将等待队列中的所有进程与因“信号量”而进入等待队列的进程区分开来，这样在唤醒的时候唤醒的是与因“信号量”不满足而睡眠的进程。


信号量的V操作相当于ucore的__up函数，代码以及相应的注释说明如下：

```
static __noinline void __up(semaphore_t *sem, uint32_t wait_state) {
    bool intr_flag;
    local_intr_save(intr_flag);
    {
        wait_t *wait;

        //若信号量中的等待队列没有进程，则直接将value+1，返回
        if ((wait = wait_queue_first(&(sem->wait_queue))) == NULL) {
            sem->value ++;
        }
        //若信号量中的等待队列有进程，则将该进程从信号量的等待队列中移出并唤醒。此处value不用+1，因为资源由本进程移交给了将被唤醒的进程。
        else {
            assert(wait->proc->wait_state == wait_state);

            //包含两个操作:1.wait_queue_del从信号量的等待队列中移出一个进程 2.wakeup_proc唤醒移出的进程
            wakeup_wait(&(sem->wait_queue), wait, wait_state, 1);
        }
    }
    local_intr_restore(intr_flag);
}
```

###哲学家问题的信号量实现
哲学家问题的信号量实现如下：

```
	sem_init(&mutex, 1);
    for(i=0;i<N;i++){
        sem_init(&s[i], 0);
        int pid = kernel_thread(philosopher_using_semaphore, (void *)i, 0);
        if (pid <= 0) {
            panic("create No.%d philosopher_using_semaphore failed.\n");
        }
        philosopher_proc_sema[i] = find_proc(pid);
        set_proc_name(philosopher_proc_sema[i], "philosopher_sema_proc");
    }
```
初始化N个哲学家进程，mutex被设置成1，这样可以实现phi_take_forks_sema、phi_put_forks_sema函数在同一时间只会被1个哲学家进程执行。

哲学家进程代码如下：

```
int philosopher_using_semaphore(void * arg) /* i：哲学家号码，从0到N-1 */
{
    int i, iter=0;
    i=(int)arg;
    cprintf("I am No.%d philosopher_sema\n",i);
    while(iter++<TIMES)
    { /* 无限循环 */
        cprintf("Iter %d, No.%d philosopher_sema is thinking\n",iter,i); /* 哲学家正在思考 */
        do_sleep(SLEEP_TIME);
        phi_take_forks_sema(i); 
        /* 需要两只叉子，或者阻塞 */
        cprintf("Iter %d, No.%d philosopher_sema is eating\n",iter,i); /* 进餐 */
        do_sleep(SLEEP_TIME);
        phi_put_forks_sema(i); 
        /* 把两把叉子同时放回桌子 */
    }
    cprintf("No.%d philosopher_sema quit\n",i);
    return 0;    
}
```
哲学家思考一段时间后，将调用phi_take_forks_sema函数：

```
void phi_take_forks_sema(int i) /* i：哲学家号码从0到N-1 */
{ 
        down(&mutex); /* 进入临界区 */
        state_sema[i]=HUNGRY; /* 记录下哲学家i饥饿的事实 */
        phi_test_sema(i); /* 试图得到两只叉子 */
        up(&mutex); /* 离开临界区 */
        down(&s[i]); /* 如果得不到叉子就阻塞 */
}
```
将哲学家标记为饥饿，进而调用phi_test_sema函数：

```
void phi_test_sema(i) /* i：哲学家号码从0到N-1 */
{ 
    if(state_sema[i]==HUNGRY&&state_sema[LEFT]!=EATING
            &&state_sema[RIGHT]!=EATING)
    {
        state_sema[i]=EATING;
        up(&s[i]);
    }
}

```
测试本哲学家是否能拿起两只叉子，注意到s[i]均初始化为0，也就是说若哲学家不满足phi_test_sema函数中的(state_sema[LEFT]!=EATING
&&state_sema[RIGHT]!=EATING)的判断，即不能拿起叉子，则在phi_take_forks_sema函数中的down(&s[i])处将被阻塞。什么时候被唤醒呢？在phi_put_forks_sema函数中：

```
void phi_put_forks_sema(int i) /* i：哲学家号码从0到N-1 */
{ 
        down(&mutex); /* 进入临界区 */
        state_sema[i]=THINKING; /* 哲学家进餐结束 */
        phi_test_sema(LEFT); /* 看一下左邻居现在是否能进餐 */
        phi_test_sema(RIGHT); /* 看一下右邻居现在是否能进餐 */
        up(&mutex); /* 离开临界区 */
}
```
当有哲学家用完餐时，将本哲学家标记为思考状态，进而调用phi_test_sema测试其左右邻居是否能拿起叉子，若有哲学家满足条件则将调用up(&s[i])，这时阻塞在phi_take_forks_sema函数中的down(&s[i])处的哲学家将被唤醒，进而调用phi_put_forks_sema而用餐。



##【1.2】请在实验报告中给出给用户态进程/线程提供信号量机制的设计方案，并比较说明给内核级提供信号量机制的异同。
###设计方案一：
将内核级信号量使用接口——P操作、V操作、init操作暴露给用户态，即作为一种系统调用的API供用户程序调用。

相同点：

*	与内核信号量的实现相同

与内核级信号量机制的不同点：

*	P操作与V操作都要从用户态进入内核态进行相应处理，然后从内核态返回，开销较大

###设计方案二：
操作系统提供原子指令test_and_set_bit、test_and_clear_bit，用户层自行设计同步互斥机制，实现信号量

不同点：

*	ucore内核使用的是控制是否允许中断实现信号量，用户程序则使用test_and_set_bit指令

###设计方案三：
操作系统提供系统调用——开中断与关中断，在用户层设计同步互斥机制，实现信号量。但是其实开中断与关中断属于特权指令，暴露出来会使得系统处于不安全状态，故不是一个好的方案。

不同点：

*	每次开中断与关中断都有内核态与用户态的切换，内核级信号量则始终在内核态。



#练习2: 完成内核级条件变量和基于内核级条件变量的哲学家就餐问题

##【2.1】请在实验报告中给出内核级条件变量的设计描述，并说其大致执行流流程。

###内核级条件变零的设计描述
管程monitor与条件变量condvar的结构与说明如下：

```
typedef struct condvar{
    semaphore_t sem;        // 记录睡在该条件变量上的进程
    int count;              // 在这个条件上睡眠的进程的个数
    monitor_t * owner;      // 此条件变量由哪个管程拥有
} condvar_t;
typedef struct monitor{
    semaphore_t mutex;      // 实现每次只允许一个进程进入管程的信号量，其value为1
    semaphore_t next;       // 记录因发出signal而进入睡眠的进程
    int next_count;         // 由于发出singal而睡眠的进程个数
    condvar_t *cv;          // 条件变量数组
} monitor_t;
```

管程的signal函数的实现与说明如下：

```
void 
cond_signal (condvar_t *cvp) {
   //LAB7 EXERCISE1: 2013011363
   cprintf("cond_signal begin: cvp %x, cvp->count %d, cvp->owner->next_count %d\n", cvp, cvp->count, cvp->owner->next_count);  
   if(cvp->count>0) {//若当前有睡在该条件变量上的进程，则唤醒
      cvp->owner->next_count ++;//由于发出signal而睡眠的进程个数+1
      up(&(cvp->sem));//唤醒睡在该条件变量上的一个进程
      down(&(cvp->owner->next));//使该进程进入睡眠，记录在因发出signal而睡眠的信号量上
      cvp->owner->next_count --;//被唤醒之后，由于发出signal而睡眠的进程个数-1
    }
   cprintf("cond_signal end: cvp %x, cvp->count %d, cvp->owner->next_count %d\n", cvp, cvp->count, cvp->owner->next_count);
}
```

管程的wait函数的实现与说明如下：

```
void
cond_wait (condvar_t *cvp) {
    //LAB7 EXERCISE1: 2013011363
    cprintf("cond_wait begin:  cvp %x, cvp->count %d, cvp->owner->next_count %d\n", cvp, cvp->count, cvp->owner->next_count);
    //进程执行了wait函数，表明此进程等待的某个条件不为真，需要睡眠
    cvp->count++; //在这个条件上睡眠的进程个数+1
    if(cvp->owner->next_count > 0)//优先唤醒因发出signal而睡在该条件变量上的进程
      up(&(cvp->owner->next));
    else
      //若没有发出signal_cv而睡眠的进程，则开放管程，使得下一个进程可以访问管程所管理的函数
      up(&(cvp->owner->mutex));
    down(&(cvp->sem));//使得此进程睡在这个条件变量上
    cvp->count --;//当此进程被唤醒，则在这个条件上睡眠的进程个数-1
    cprintf("cond_wait end:  cvp %x, cvp->count %d, cvp->owner->next_count %d\n", cvp, cvp->count, cvp->owner->next_count);
}
```
与课上讲的原理有较大差异，一是通过信号量进行实现锁机制，二是多记录了一个由于发出signal而进入睡眠的进程队列，实现了Hoare管程，即signal发出后立刻唤醒等待进程，使正在执行的进程进入睡眠。

###哲学家问题的管程实现
哲学家问题的管程实现如下：

```
//check condition variable
    monitor_init(&mt, N);//管程中有N个条件变量，对应N个哲学家；N个条件变量中的信号量的value初始化为0
    for(i=0;i<N;i++){
        state_condvar[i]=THINKING;
        int pid = kernel_thread(philosopher_using_condvar, (void *)i, 0);
        if (pid <= 0) {
            panic("create No.%d philosopher_using_condvar failed.\n");
        }
        philosopher_proc_condvar[i] = find_proc(pid);
        set_proc_name(philosopher_proc_condvar[i], "philosopher_condvar_proc");
    }
```
初始化1个管程、N个哲学家进程、N个对应的条件变量，条件变量中的信号量的value为0，对应信号量实现中的s数组，管程中的mutex初始化为1，通过信号量的使用实现锁机制。

哲学家进程代码如下：

```
int philosopher_using_condvar(void * arg) { /* arg is the No. of philosopher 0~N-1*/
  
    int i, iter=0;
    i=(int)arg;
    cprintf("I am No.%d philosopher_condvar\n",i);
    while(iter++<TIMES)
    { /* iterate*/
        cprintf("Iter %d, No.%d philosopher_condvar is thinking\n",iter,i); /* thinking*/
        do_sleep(SLEEP_TIME);
        phi_take_forks_condvar(i); //拿到两只叉子
        /* need two forks, maybe blocked */
        cprintf("Iter %d, No.%d philosopher_condvar is eating\n",iter,i); /* eating*/
        do_sleep(SLEEP_TIME);
        phi_put_forks_condvar(i); //释放两只叉子，并试图唤醒左右邻居进餐
        /* return two forks back*/
    }
    cprintf("No.%d philosopher_condvar quit\n",i);
    return 0;    
}
```
与信号量的实现一样，哲学家思考一段时间后，将调用phi_take_forks_condvar函数试图获得两只叉子：

```
void phi_take_forks_condvar(int i) {
     down(&(mtp->mutex));   //进入临界区
//--------into routine in monitor--------------
     // LAB7 EXERCISE1: 2013011363
     // I am hungry
     // try to get fork

    state_condvar[i]=HUNGRY; //记录下哲学家i饥饿的事实
    phi_test_condvar(i); //试图得到两只叉子
    while (state_condvar[i] != EATING) {//注：信号量的实现中，不需要这个循环
        cprintf("phi_take_forks_condvar: %d didn't get fork and will wait\n",i);
        cond_wait(&mtp->cv[i]);//若未得到两只叉子，则阻塞，等待被其邻居唤醒
    }
//--------leave routine in monitor--------------
    //当不存在因发出signal而进入睡眠的进程时，才释放Mutex资源，使得其他进程能够访问管程控制的函数，即phi_take_forks_condvar与phi_put_forks_condvar
    if(mtp->next_count>0)
        up(&(mtp->next));
    else
        up(&(mtp->mutex));
}
```
将哲学家标记为饥饿状态，进而调用phi_test_condvar函数测试是否能拿起两只叉子，若不能，则state_condvar[i]的值未赋值为EATING，意味着本哲学家进程将阻塞在while (state_condvar[i] != EATING)循环中，直到其他哲学家用餐完毕，调用phi_put_forks_condvar函数放下两把叉子并试图唤醒被阻塞的哲学家进程。

```
void phi_put_forks_condvar(int i) {
     down(&(mtp->mutex));

//--------into routine in monitor--------------
     // LAB7 EXERCISE1: 2013011363
     // I ate over
     // test left and right neighbors

      state_condvar[i]=THINKING;//状态标记为思考
      phi_test_condvar(LEFT);//试图唤醒左邻居进餐
      phi_test_condvar(RIGHT);//试图唤醒右邻居进餐
//--------leave routine in monitor--------------
     if(mtp->next_count>0)
        up(&(mtp->next));
     else
        up(&(mtp->mutex));
}
```

注意到phi_take_forks_condvar与phi_put_forks_condvar函数的结尾都有

```
	if(mtp->next_count>0)
        up(&(mtp->next));
    else
        up(&(mtp->mutex));
```
的代码块，只有当不存在因发出signal而进入睡眠的进程时，才释放Mutex资源，使得其他进程能够访问管程控制的函数，即phi_take_forks_condvar与phi_put_forks_condvar。



##【2.2】请在实验报告中给出给用户态进程/线程提供条件变量机制的设计方案，并比较说明给内核级提供条件变量机制的异同。

###设计方案一：
将管程暴露给用户态，即作为一种系统调用的API供用户程序调用。

相同点：

*	与内核信号量的实现相同

与内核级信号量机制的不同点：

*	P操作与V操作都要从用户态进入内核态进行相应处理，然后从内核态返回，开销较大


###设计方案二：
操作系统提供原子指令test_and_set_bit、test_and_clear_bit，用户层自行设计同步互斥机制，实现管程

不同点：

*	ucore内核使用信号量实现管程，用户程序则使用test_and_set_bit指令

###设计方案三：
操作系统提供系统调用——开中断与关中断，在用户层设计同步互斥机制，实现管程。但是其实开中断与关中断属于特权指令，暴露出来会使得系统处于不安全状态，故不是一个好的方案。

不同点：

*	每次开中断与关中断都有内核态与用户态的切换，内核级管程则始终在内核态。











