// Sleeping locks

#include "inc/types.h"
#include "inc/x86.h"
#include "inc/memlayout.h"
#include "inc/mmu.h"
#include "inc/environment_definitions.h"
#include "inc/assert.h"
#include "inc/string.h"
#include "sleeplock.h"
#include "channel.h"
#include "../cpu/cpu.h"
#include "../proc/user_environment.h"

void init_sleeplock(struct sleeplock *lk, char *name)
{
	init_channel(&(lk->chan), "sleep lock channel");
	init_spinlock(&(lk->lk), "lock of sleep lock");
	strcpy(lk->name, name);
	lk->locked = 0;
	lk->pid = 0;
}
int holding_sleeplock(struct sleeplock *lk)
{
	int r;
	acquire_spinlock(&(lk->lk));
	r = lk->locked && (lk->pid == get_cpu_proc()->env_id);
	release_spinlock(&(lk->lk));
	return r;
}
//==========================================================================

void acquire_sleeplock(struct sleeplock *lk)
{
	//TODO: [PROJECT'24.MS1 - #13] [4] LOCKS - acquire_sleeplock
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("acquire_sleeplock is not implemented yet");
	//Your Code is Here...


	if(holding_sleeplock(lk)) // check the process is holding the lock
	     panic("acquire_sleeplock: lock \"%s\" is already held by the same CPU.", lk->name);

   acquire_spinlock(&(lk->lk)); //acquire guard
    while(lk->locked == 1) // process check the lock if busy : ( go to sleep )  else : (process go to the critical section & set lock = busy)
    {
    	sleep(&lk->chan , &lk->lk); // process go to sleep
    }
    lk->locked = 1; // Busy
    //asm volatile("movl $1, %0" : "+m" (lk->locked) : ); // Busy in assembly
    lk->pid = get_cpu_proc()->env_id; // set process id = current running process id
    release_spinlock(&(lk->lk)); //release guard

}
void release_sleeplock(struct sleeplock *lk)
{
	//TODO: [PROJECT'24.MS1 - #14] [4] LOCKS - release_sleeplock
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("release_sleeplock is not implemented yet");
	//Your Code is Here...

	if(!holding_sleeplock(lk)) // check the lock held by same cpu (process)
		panic("release: lock \"%s\" is either not held or held by another CPU!", lk->name);
	 acquire_spinlock(&(lk->lk)); // acquire guard

	 if(queue_size(&lk->chan.queue) != 0) // check if any process in wait queue if exist : move all process to ready queue
	 {
		 wakeup_all(&lk->chan); //move all process to ready queue
	 }

	 lk->locked = 0; // free
	// asm volatile("movl $0, %0" : "+m" (lk->locked) : );//free in assembly
	 lk->pid = 0;// set process id
	 release_spinlock(&(lk->lk)); // release guard

}




