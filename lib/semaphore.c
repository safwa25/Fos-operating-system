// User-level Semaphore

#include "inc/lib.h"

struct semaphore create_semaphore(char *semaphoreName, uint32 value)
{
	//TODO: [PROJECT'24.MS3 - #02] [2] USER-LEVEL SEMAPHORE - create_semaphore
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("create_semaphore is not implemented yet");
	//Your Code is Here...
    // cprintf("in create !\n");
	 struct semaphore new_semaphore;

	 struct __semdata * SemData = (struct __semdata *)smalloc(semaphoreName, sizeof(struct __semdata), 1);

	 if (SemData == NULL)
	 {
		 new_semaphore.semdata = SemData;
		return new_semaphore;
	 }

	 SemData->count = value;
	 SemData->lock = 0;
	 strcpy(SemData->name, semaphoreName);
	 sys_initQueue(&SemData->queue);

	 new_semaphore.semdata = SemData;
	 return new_semaphore;

}
struct semaphore get_semaphore(int32 ownerEnvID, char* semaphoreName)
{
	//TODO: [PROJECT'24.MS3 - #03] [2] USER-LEVEL SEMAPHORE - get_semaphore
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("get_semaphore is not implemented yet");
	//Your Code is Here...
	//cprintf("in get !\n");

	struct semaphore sem ;

	struct __semdata * SemData = (struct __semdata *)sget(ownerEnvID,semaphoreName);

	if(SemData == NULL)
	{
	   sem.semdata = SemData;
	   return sem;
	}

	sem.semdata = SemData;

	return sem;
}

void wait_semaphore(struct semaphore sem)
{
	//TODO: [PROJECT'24.MS3 - #04] [2] USER-LEVEL SEMAPHORE - wait_semaphore
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("wait_semaphore is not implemented yet");
	//Your Code is Here...

	while(xchg(&(sem.semdata->lock), 1) != 0) ;

    sem.semdata->count--;
    if(sem.semdata->count < 0)
    {
    	sys_GoToSleep(sem.semdata);
    }
    else
    {
     sem.semdata->lock = 0;
    }

}

void signal_semaphore(struct semaphore sem)
{
	//TODO: [PROJECT'24.MS3 - #05] [2] USER-LEVEL SEMAPHORE - signal_semaphore
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("signal_semaphore is not implemented yet");
	//Your Code is Here...

	while(xchg(&(sem.semdata->lock), 1) != 0) ;

	sem.semdata->count++;

	if(sem.semdata->count <= 0)
	{
		sys_GoToReady(&sem.semdata->queue);
	}

	sem.semdata->lock = 0;
}

int semaphore_count(struct semaphore sem)
{
	return sem.semdata->count;
}
