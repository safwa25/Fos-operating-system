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
	int NthClk_WS_Size = 20 ;
	int ID_NORMAL_1, ID_NORMAL_10, ID_MODIFIED_10;

	//Nth Clock NORMAL (N = 1)
	char setNthClkCmd1[100] = "__NthClkRepl@1";
	sys_utilities(setNthClkCmd1, 1);
	{
		rsttst();
		ID_NORMAL_1 = sys_create_env("sc_qs_leak", NthClk_WS_Size, 0, 0);
		if (ID_NORMAL_1 == E_ENV_CREATION_ERROR)
			panic("RUNNING OUT OF ENV!! terminating...");
		sys_run_env(ID_NORMAL_1);

		//Wait until the first program end
		while (gettst() != 1) ;
	}

	//Nth Clock NORMAL (N = 10)
	char setNthClkCmd2[100] = "__NthClkRepl@1";
	sys_utilities(setNthClkCmd2, 10);
	{
		rsttst();
		ID_NORMAL_10 = sys_create_env("sc_qs_leak", NthClk_WS_Size, 0, 0);
		if (ID_NORMAL_10 == E_ENV_CREATION_ERROR)
			panic("RUNNING OUT OF ENV!! terminating...");
		sys_run_env(ID_NORMAL_10);

		//Wait until the first program end
		while (gettst() != 1) ;
	}

	//Nth Clock MODIFIED (N = 10)
	char setNthClkCmd3[100] = "__NthClkRepl@2";
	sys_utilities(setNthClkCmd3, 10);
	{
		rsttst();
		ID_MODIFIED_10 = sys_create_env("sc_qs_leak", NthClk_WS_Size, 0, 0);
		if (ID_MODIFIED_10 == E_ENV_CREATION_ERROR)
			panic("RUNNING OUT OF ENV!! terminating...");
		sys_run_env(ID_MODIFIED_10);

		//Wait until the first program end
		while (gettst() != 1) ;
	}
	//VALIDATE Results
	{
		volatile struct Env* env_NORMAL_1 = NULL ;
		//envid2env(ID_FIFO, &env_FIFO, 0);
		env_NORMAL_1 = &envs[ENVX(ID_NORMAL_1)];
		assert(env_NORMAL_1->env_id == ID_NORMAL_1) ;

		volatile struct Env* env_NORMAL_10 = NULL ;
		//envid2env(ID_LRU, &env_LRU, 0);
		env_NORMAL_10 = &envs[ENVX(ID_NORMAL_10)];
		assert(env_NORMAL_10->env_id == ID_NORMAL_10) ;

		volatile struct Env* env_MODIFIED_10 = NULL ;
		//envid2env(ID_LRU, &env_LRU, 0);
		env_MODIFIED_10 = &envs[ENVX(ID_MODIFIED_10)];
		assert(env_MODIFIED_10->env_id == ID_MODIFIED_10) ;

		if ((env_NORMAL_1->nPageIn > env_NORMAL_10->nPageIn) && (env_NORMAL_1->nPageOut > env_NORMAL_10->nPageOut) &&
			(env_NORMAL_10->nPageIn > env_MODIFIED_10->nPageIn) && (env_NORMAL_10->nPageOut > env_MODIFIED_10->nPageOut) &&
			(env_NORMAL_1->nNewPageAdded == env_NORMAL_10->nNewPageAdded) && (env_NORMAL_10->nNewPageAdded == env_MODIFIED_10->nNewPageAdded))
		{
			//cprintf("%~\n\nCongratulations... Nth Clock: Normal vs Modified scenario finished\n\n");
			atomic_cprintf("%~\nCongratulations!!... test is completed.\n");
		}
		else
		{
			panic("%~Unexpected result: the number of PageIn/PageOut of Normal Version is expected to be greater than the Modified Version ones\nNumber of Newly Added Pages are expected to be equal\n");
		}
	}

}
