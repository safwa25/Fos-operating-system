/*
 * channel.c
 *
 *  Created on: Sep 22, 2024
 *      Author: HP
 */
#include "channel.h"
#include <kern/proc/user_environment.h>
#include <kern/cpu/sched.h>
#include <inc/string.h>
#include <inc/disk.h>

//===============================
// 1) INITIALIZE THE CHANNEL:
//===============================
// initialize its lock & queue
void init_channel(struct Channel *chan, char *name)
{
	strcpy(chan->name, name);
	init_queue(&(chan->queue));
}

//===============================
// 2) SLEEP ON A GIVEN CHANNEL:
//===============================
// Atomically release lock and sleep on chan.
// Reacquires lock when awakened.
// Ref: xv6-x86 OS code
void sleep(struct Channel *chan, struct spinlock* lk)
{
	//TODO: [PROJECT'24.MS1 - #10] [4] LOCKS - sleep
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("sleep is not implemented yet");
	//Your Code is Here...

	acquire_spinlock(&ProcessQueues.qlock); // acquire lock queues of process
    release_spinlock(lk); // release guard

    struct Env* _current_process = get_cpu_proc(); // get current process
    assert(_current_process != NULL); // check that the current process is not null
    _current_process->env_status = ENV_BLOCKED; // change status of the process to blocked
    //get_cpu_proc()->env_status = ENV_BLOCKED;
    enqueue(&chan->queue , _current_process); // insert current process to wait queue
    sched(); // if any process in ready queue set status is running and go to cpu
    if(get_cpu_proc()->env_status == ENV_RUNNING) // process wakeup
        acquire_spinlock(lk); // reacquire guard

    release_spinlock(&ProcessQueues.qlock); // release the lock of queues

}
//==================================================
// 3) WAKEUP ONE BLOCKED PROCESS ON A GIVEN CHANNEL:
//==================================================
// Wake up ONE process sleeping on chan.
// The qlock must be held.
// Ref: xv6-x86 OS code
// chan MUST be of type "struct Env_Queue" to hold the blocked processes
void wakeup_one(struct Channel *chan)
{
	//TODO: [PROJECT'24.MS1 - #11] [4] LOCKS - wakeup_one
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("wakeup_one is not implemented yet");
	//Your Code is Here...

	   acquire_spinlock(&ProcessQueues.qlock); // acquire lock queues of process

	   if(queue_size(&chan->queue) != 0) // check if any process in wait queue (wakeup one process)
	   {
	    struct Env* mv_process_to_readyQueue = dequeue(&chan->queue); // get process from wait queue
	    assert(mv_process_to_readyQueue != NULL);// check that the current process is not null
	    sched_insert_ready(mv_process_to_readyQueue); // inside this function (process status changed to ENV_READY)
 	   }

	   release_spinlock(&ProcessQueues.qlock);// release the lock of queues

}

//====================================================
// 4) WAKEUP ALL BLOCKED PROCESSES ON A GIVEN CHANNEL:
//====================================================
// Wake up all processes sleeping on chan.
// The queues lock must be held.
// Ref: xv6-x86 OS code
// chan MUST be of type "struct Env_Queue" to hold the blocked processes

void wakeup_all(struct Channel *chan)
{
	//TODO: [PROJECT'24.MS1 - #12] [4] LOCKS - wakeup_all
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("wakeup_all is not implemented yet");
	//Your Code is Here...


	   acquire_spinlock(&ProcessQueues.qlock); // acquire lock queues of process

	   if(queue_size(&chan->queue) != 0)
	   {

	    int size = queue_size(&chan->queue);
	    struct Env* mv_process_to_readyQueue;

	    for (int i = 0; i < size; i++) { // wake up all processes in wait queue

	    	mv_process_to_readyQueue = dequeue(&chan->queue);
	    	assert(mv_process_to_readyQueue != NULL);// check that the current process is not null
	        sched_insert_ready(mv_process_to_readyQueue);// inside this function (process status changed to ENV_READY)

	       }
	   }
	   release_spinlock(&ProcessQueues.qlock);// release the lock of queues
}
