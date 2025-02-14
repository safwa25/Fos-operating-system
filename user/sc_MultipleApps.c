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
	int WS_Size = 10000 ;
	int ID_MatOps, ID_Fact;
	int* ID_Fibs = malloc(3*sizeof(int));

	//setPageReplacmentAlgorithmNchanceCLOCK(1);
	sys_utilities("__ReplStrat__", -0x6);
	rsttst();

	//Matrix Operations: 9 trials in same program
	{
		ID_MatOps = sys_create_env("sc_matops", WS_Size, 0, 0);
		if (ID_MatOps == E_ENV_CREATION_ERROR)
			panic("RUNNING OUT OF ENV!! terminating...");
	}

	//Factorial: 1 program
	{
		ID_Fact = sys_create_env("sc_fact_recursive", WS_Size, 0, 0);
		if (ID_Fact == E_ENV_CREATION_ERROR)
			panic("RUNNING OUT OF ENV!! terminating...");
	}

	//Fibonacci: 3 programs (recursive, loop & memomization)
	{
		ID_Fibs[0] = sys_create_env("sc_fib_recursive", WS_Size, 0, 0);
		ID_Fibs[1] = sys_create_env("sc_fib_loop", WS_Size, 0, 0);
		ID_Fibs[2] = sys_create_env("sc_fib_memomize", WS_Size, 0, 0);

		for (int i = 0; i < 3; ++i)
		{
			if (ID_Fibs[i] == E_ENV_CREATION_ERROR)
				panic("RUNNING OUT OF ENV!! terminating...");
		}
	}

	//RUN ALL
	{
		sys_run_env(ID_MatOps);
		sys_run_env(ID_Fact);
		for (int i = 0; i < 3; ++i)
		{
			sys_run_env(ID_Fibs[i]);
		}
	}

	while (gettst() != 5);

	//VALIDATE Results
	{
		volatile struct Env* env_MatOps = NULL ;
		//envid2env(ID_FIFO, &env_LEAK, 0);
		env_MatOps = &envs[ENVX(ID_MatOps)];
		assert(env_MatOps->env_id == ID_MatOps) ;

		volatile struct Env* env_Fact = NULL ;
		//envid2env(ID_LRU, &env_LRU, 0);
		env_Fact = &envs[ENVX(ID_Fact)];
		assert(env_Fact->env_id == ID_Fact) ;

		volatile struct Env** env_Fibs = malloc(3*sizeof(struct Env*));
		for (int i = 0; i < 3; ++i)
		{
			env_Fibs[i] = &envs[ENVX(ID_Fibs[i])];
			assert(env_Fibs[i]->env_id == ID_Fibs[i]) ;
		}
		free(ID_Fibs) ;

		bool FactCheck = (env_Fact->nPageIn == 0) && (env_Fact->nPageOut == 0) ? 1 : 0;
		bool MatOpsCheck = (env_MatOps->nPageIn == 0) && (env_MatOps->nPageOut == 0) ? 1 : 0;
		bool FibCheck = (env_Fibs[0]->pageFaultsCounter < env_Fibs[1]->pageFaultsCounter) && (env_Fibs[1]->pageFaultsCounter < env_Fibs[2]->pageFaultsCounter)? 1 : 0;
		FibCheck = FibCheck && (env_Fibs[0]->nPageIn == 0) && (env_Fibs[1]->nPageIn == 0) && (env_Fibs[2]->nPageIn == 0);
		FibCheck = FibCheck && (env_Fibs[0]->nPageOut== 0) && (env_Fibs[1]->nPageOut == 0) && (env_Fibs[2]->nPageOut == 0);
		if (FactCheck && MatOpsCheck && FibCheck)
		{
			free(env_Fibs) ;
			//cprintf("Congratulations... MULTIPLE APPS scenario finished\n");
			atomic_cprintf("%~\nCongratulations!!... test is completed.\n");
		}
		else
		{
			free(env_Fibs) ;
			panic("Unexpected result\n");
		}
	}

}
