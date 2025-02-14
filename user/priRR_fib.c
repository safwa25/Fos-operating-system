
#include <inc/lib.h>


int fibonacci(int n);

void
_main(void)
{
	int i1=0;
	char buff1[256];
	i1 = 38;

	int res = fibonacci(i1) ;

	atomic_cprintf("Fibonacci #%d = %d\n",i1, res);

	if (res != 63245986)
		panic("[envID %d] wrong result!", myEnv->env_id);

	//To indicate that it's completed successfully
	inctst();

	return;
}


int fibonacci(int n)
{
	if (n <= 1)
		return 1 ;
	return fibonacci(n-1) + fibonacci(n-2) ;
}

