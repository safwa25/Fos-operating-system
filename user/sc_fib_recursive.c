
#include <inc/lib.h>


int64 fibonacci(int n);

void
_main(void)
{
	int index=0;
	char buff1[256];
//	atomic_readline("Please enter Fibonacci index:", buff1);
//	i1 = strtol(buff1, NULL, 10);

	index = 30;

	int64 res = fibonacci(index) ;

	atomic_cprintf("Fibonacci #%d = %lld\n",index, res);
	//To indicate that it's completed successfully
	inctst();
	return;
}


int64 fibonacci(int n)
{
	if (n <= 1)
		return 1 ;
	return fibonacci(n-1) + fibonacci(n-2) ;
}

