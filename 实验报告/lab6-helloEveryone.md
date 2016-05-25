#2013011363 黄予 计34
#练习1: 使用 Round Robin 调度算法
##【1.1】请理解并分析sched_calss中各个函数指针的用法，并接合Round Robin 调度算法描ucore的调度执行过程

分析RR算法的执行流程如下：

每来一次时钟中断，在trap.c中调用sched_class_proc_tick函数，其定义如下：

```
void
sched_class_proc_tick(struct proc_struct *proc) {
    if (proc != idleproc) {
        sched_class->proc_tick(rq, proc);
    }
    else {
        proc->need_resched = 1;
    }
}
```
进而调用sched_class->proc_tick，即：

```
static void
RR_proc_tick(struct run_queue *rq, struct proc_struct *proc) {
    if (proc->time_slice > 0) {//减少可运行时间片
        proc->time_slice --;
    }
    if (proc->time_slice == 0) {//当可运行时间片为0时，need_resched置1
        proc->need_resched = 1;
    }
}
```
在该函数中，当前进程的可运行时间片减少1，若当前进程的可运行时间片已经为0，则将当前进程的need_resched置1，这样在下一次中断的时候，在trap.c的trap()函数中通过：

```
	if (current->need_resched) {
		schedule();
	}
```
进行调度，进而调用schedule()函数：

```
void
schedule(void) {
    bool intr_flag;
    struct proc_struct *next;
    local_intr_save(intr_flag);
    {
        current->need_resched = 0;
        if (current->state == PROC_RUNNABLE) {
            sched_class_enqueue(current);
        }
        if ((next = sched_class_pick_next()) != NULL) {
            sched_class_dequeue(next);
        }
        if (next == NULL) {
            next = idleproc;
        }
        next->runs ++;
        if (next != current) {
            proc_run(next);
        }
    }
    local_intr_restore(intr_flag);
}
```
经过以下3步进行进程切换：
1.	当前进程进入就绪进程的队列，即调用sched_class_enqueue（）：

```
static void
RR_enqueue(struct run_queue *rq, struct proc_struct *proc) {
    assert(list_empty(&(proc->run_link)));
    list_add_before(&(rq->run_list), &(proc->run_link));//将新进程插入队尾
    if (proc->time_slice == 0 || proc->time_slice > rq->max_time_slice) {//若进程的时间已经用完，则重置可运行时间
        proc->time_slice = rq->max_time_slice;
    }
    proc->rq = rq;
    rq->proc_num ++;//排队的进程数增加1        
}
```
在RR算法中，直接将进程插入队尾即可

2.	调用sched_class_pick_next()选择下一个应该执行的进程：

```
static struct proc_struct *
RR_pick_next(struct run_queue *rq) {
    list_entry_t *le = list_next(&(rq->run_list));//取出位于队头的进程
    if (le != &(rq->run_list)) {
        return le2proc(le, run_link);
    }
    return NULL;
}
```
在RR算法中，直接取出位于就绪队列队头的进程作为下一个执行的进程

3.	调用ched_class_dequeue(next)，使被选择的进程出队：

```
static void
RR_dequeue(struct run_queue *rq, struct proc_struct *proc) {
    assert(!list_empty(&(proc->run_link)) && proc->rq == rq);
    list_del_init(&(proc->run_link));//从就绪队列中取出某进程       
    rq->proc_num --;//排队的进程数减1
}
```
最后调用proc_run()函数执行被选择的进程。

##【1.2】在实验报告中简要说明如何设计实现”多级反馈队列调度算法“，给出概要设计

RR算法中只维护了一个队列run_queue，多级反馈队列调度算法则需要维护多个队列，并且run_queue类中还需要增加一个属性，表示队列的优先级，或者该队列的最大可执行时间片，几个函数的设计如下：

1.	pick_next函数：从优先级高的队列开始取进程，仅当高优先级的队列中无进程可取才查询低优先级队列
2.	proc_tick函数：与RR算法的实现大致相同，即每次时钟中断则将time_slice减1，若减至0则将need_resched置1
3.	enqueue函数：将进程插入指定队列的队尾
4.	dequeue函数：将指定队列的头部的进程取出
5.	schedule函数：若当前进程的时间片用完，则调用enqueue将其放入更低优先级的队列，若队列不存在则创建；调用pick_next选择下一个执行的进程；调用dequeue取出下一个执行的进程。


#练习2: 实现 Stride Scheduling 调度算法

##【1.1】设计实现过程
使用链表作为存储就绪进程的数据结构，enqueue、dequeue、stride_proc_tick函数与RR算法一致，重点为stride_pick_next函数：

```
static struct proc_struct *
stride_pick_next(struct run_queue *rq) {
    //遍历就绪队列，找出stride最小的进程
    list_entry_t *le = list_next(&(rq->run_list));

    if (le == &rq->run_list) //若就绪队列为空，则直接返回NULL
        return NULL;

    struct proc_struct *p = le2proc(le, run_link);
    le = list_next(le);
    //遍历
    while (le != &rq->run_list){
        struct proc_struct *temp = le2proc(le, run_link);
        if ((int32_t)(temp->lab6_stride - p->lab6_stride) < 0)
            p = temp;
        le = list_next(le);
    }

    //更新stride
    if (p->lab6_priority == 0)
        p->lab6_stride += BIG_STRIDE;
    else 
        p->lab6_stride += BIG_STRIDE / p->lab6_priority;
    return p;
```
思路为遍历链表中的所有进程，找出stride最小的进程作为下一个运行的进程，按stride算法更新选择进程的stride。在这里，若是(int32_t)(a_stride - b_stride) < 0，则认为进程a的stride小于进程b。具体分析在课上做过，可见[stride溢出问题](https://github.com/yu-huang13/OS_HY/blob/master/06-2-lab6-spoc/report.md).












