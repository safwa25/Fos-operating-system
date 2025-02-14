
#include <inc/lib.h>

//extern void sys_env_set_priority(int , int );

void
_main(void)
{
	int32 envIdFib1 = sys_create_env("priRR_fib", (myEnv->page_WS_max_size),(myEnv->SecondListSize), (myEnv->percentage_of_WS_pages_to_be_removed));
	if (envIdFib1 == E_ENV_CREATION_ERROR)
		panic("Loading programs failed\n");

	int32 envIdFib2 = sys_create_env("priRR_fib", (myEnv->page_WS_max_size),(myEnv->SecondListSize), (myEnv->percentage_of_WS_pages_to_be_removed));
	if (envIdFib2 == E_ENV_CREATION_ERROR)
		panic("Loading programs failed\n");

	sys_run_env(envIdFib1);
	sys_run_env(envIdFib2);

	int priority = 2;
	cprintf("process %d will be added to ready queue at priority %d\n", envIdFib1, priority);
	//sys_env_set_priority(envIdFib1, priority);

	priority = 9;
	cprintf("process %d will be added to ready queue at priority %d\n", envIdFib2, priority);
	//sys_env_set_priority(envIdFib2, priority);
return;
}


