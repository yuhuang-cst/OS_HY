#include <stdio.h>
#include <monitor.h>
#include <kmalloc.h>
#include <assert.h>


// Initialize monitor.
void     
monitor_init (monitor_t * mtp, size_t num_cv) {
    int i;
    assert(num_cv>0);
    mtp->next_count = 0;
    mtp->cv = NULL;
    sem_init(&(mtp->mutex), 1); //unlocked
    sem_init(&(mtp->next), 0);
    mtp->cv =(condvar_t *) kmalloc(sizeof(condvar_t)*num_cv);
    assert(mtp->cv!=NULL);
    for(i=0; i<num_cv; i++){
        mtp->cv[i].count=0;
        sem_init(&(mtp->cv[i].sem),0);//条件变量中的信号量的value初始化为0
        mtp->cv[i].owner=mtp;
    }
}

// Unlock one of threads waiting on the condition variable. 
void 
cond_signal (condvar_t *cvp) {
   //LAB7 EXERCISE1: 2013011363
   cprintf("cond_signal begin: cvp %x, cvp->count %d, cvp->owner->next_count %d\n", cvp, cvp->count, cvp->owner->next_count);  
  /*
   *      cond_signal(cv) {
   *          if(cv.count>0) {
   *             mt.next_count ++;
   *             signal(cv.sem);
   *             wait(mt.next);
   *             mt.next_count--;
   *          }
   *       }
   */
   if(cvp->count>0) {//若当前有睡在该条件变量上的进程，则唤醒
      cvp->owner->next_count ++;//由于发出signal而睡眠的进程个数+1
      up(&(cvp->sem));//唤醒睡在该条件变量上的一个进程
      down(&(cvp->owner->next));//使该进程进入睡眠，记录在因发出signal而睡眠的信号量上
      cvp->owner->next_count --;//被唤醒之后，由于发出signal而睡眠的进程个数-1
    }
   cprintf("cond_signal end: cvp %x, cvp->count %d, cvp->owner->next_count %d\n", cvp, cvp->count, cvp->owner->next_count);
}

// Suspend calling thread on a condition variable waiting for condition Atomically unlocks 
// mutex and suspends calling thread on conditional variable after waking up locks mutex. Notice: mp is mutex semaphore for monitor's procedures
void
cond_wait (condvar_t *cvp) {
    //LAB7 EXERCISE1: 2013011363
    cprintf("cond_wait begin:  cvp %x, cvp->count %d, cvp->owner->next_count %d\n", cvp, cvp->count, cvp->owner->next_count);
   /*
    *         cv.count ++;
    *         if(mt.next_count>0)
    *            signal(mt.next)
    *         else
    *            signal(mt.mutex);
    *         wait(cv.sem);
    *         cv.count --;
    */
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
