#include <inc/lib.h>

inline unsigned int nearest_pow2_ceil(unsigned int x)
{
	if (x <= 1) return 1;
	int power = 2;
	x--;
	while (x >>= 1) {
		power <<= 1;
	}
	return power;
}

void _main(void)
{
#define NUM_OF_INSTANCES 3
	int WS_Size = 10000 ;
	int ID_ArrOps[NUM_OF_INSTANCES];

	//setPageReplacmentAlgorithmNchanceCLOCK(1);
	sys_utilities("__ReplStrat__", -0x6);

	//CREATE SEMAPHORES
	struct semaphore arropsFinished = create_semaphore("arropsFinished", 0);
	struct semaphore cons_mutex = create_semaphore("Console Mutex", 1);

	//ArrayOperations: 3 programs
	{
		for (int i = 0; i < NUM_OF_INSTANCES; ++i)
		{
			ID_ArrOps[i] = sys_create_env("sc_arrops_slave", WS_Size, 0, 0);
			if (ID_ArrOps[i] == E_ENV_CREATION_ERROR)
				panic("RUNNING OUT OF ENV!! terminating...");
		}
	}

	//RUN ALL
	{
		for (int i = 0; i < NUM_OF_INSTANCES; ++i)
		{
			sys_run_env(ID_ArrOps[i]);
		}
	}

	//WAIT UNTIL ALL FINISHED
	for (int i = 0; i < NUM_OF_INSTANCES; ++i)
	{
		wait_semaphore(arropsFinished);
	}

	//CHECK SEMAPHORE VALUES
	assert(semaphore_count(arropsFinished) == 0);
	assert(semaphore_count(cons_mutex) == 1);

	//VALIDATE Results
	{
		volatile struct Env* env_ArrOps[NUM_OF_INSTANCES];
		for (int i = 0; i < NUM_OF_INSTANCES; ++i)
		{
			env_ArrOps[i] = &envs[ENVX(ID_ArrOps[i])];
			assert(env_ArrOps[i]->env_id == ID_ArrOps[i]) ;
		}

		bool arrOpsCheck = 1;
		arrOpsCheck = arrOpsCheck && (env_ArrOps[0]->nPageIn == 0) && (env_ArrOps[1]->nPageIn == 0) && (env_ArrOps[2]->nPageIn == 0);
		arrOpsCheck = arrOpsCheck && (env_ArrOps[0]->nPageOut== 0) && (env_ArrOps[1]->nPageOut == 0) && (env_ArrOps[2]->nPageOut == 0);
		if (arrOpsCheck)
		{
			//cprintf("Congratulations... ARRAY OPERATIONS scenario finished\n");
			atomic_cprintf("%~\nCongratulations!!... test is completed.\n");
		}
		else
		{
			panic("Unexpected result\n");
		}
	}

}
