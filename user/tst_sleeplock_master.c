// Test the Sleep Lock for applying critical section
// Master program: create and run slaves, wait them to finish
#include <inc/lib.h>

void
_main(void)
{
	int envID = sys_getenvid();
	char slavesCnt[10];
	readline("Enter the number of Slave Programs: ", slavesCnt);
	int numOfSlaves = strtol(slavesCnt, NULL, 10);

	rsttst();

	//Create and run slave programs that should sleep
	int id;
	for (int i = 0; i < numOfSlaves; ++i)
	{
		id = sys_create_env("tstSleepLockSlave", (myEnv->page_WS_max_size),(myEnv->SecondListSize), (myEnv->percentage_of_WS_pages_to_be_removed));
		if (id== E_ENV_CREATION_ERROR)
		{
			cprintf("\n%~insufficient number of processes in the system! only %d slave processes are created\n", i);
			numOfSlaves = i;
			break;
		}
		sys_run_env(id);
	}

	//Wait until all slaves, except one, are blocked
	int numOfBlockedProcesses = 0;
	sys_utilities("__GetLockQueueSize__", (uint32)(&numOfBlockedProcesses));
	int cnt = 0;
	while (numOfBlockedProcesses != numOfSlaves-1)
	{
		env_sleep(5000);
		if (cnt == numOfSlaves)
		{
			panic("%~test sleeplock failed! unexpected number of blocked slaves. Expected = %d, Current = %d", numOfSlaves-1, numOfBlockedProcesses);
		}
		sys_utilities("__GetLockQueueSize__", (uint32)(&numOfBlockedProcesses));
		cnt++ ;
	}

	//signal the slave inside the critical section to leave it
	inctst();

	//Wait until all slave finished
	cnt = 0;
	while (gettst() != numOfSlaves +1/*since master already increment it by 1*/)
	{
		env_sleep(5000);
		if (cnt == numOfSlaves)
		{
			panic("%~test sleeplock failed! not all slaves finished. Expected %d, Actual %d", numOfSlaves +1, gettst());
		}
		cnt++ ;
	}

	cprintf("%~\n\nCongratulations!! Test of Sleep Lock completed successfully!!\n\n");
}
