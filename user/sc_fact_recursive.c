
#include <inc/lib.h>


int64 factorial(int n);

void
_main(void)
{
	int n=0;
	char buff1[256];

	n = 20 ;
	atomic_cprintf("Please enter a number: %d\n", n);
	int64 res = factorial(n) ;

	atomic_cprintf("Factorial %d = %lld\n",n, res);
	//To indicate that it's completed successfully
	inctst();
	return;
}


int64 factorial(int n)
{
	if (n <= 1)
		return 1 ;
	return n * factorial(n-1) ;
}

