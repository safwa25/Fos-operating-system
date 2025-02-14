// Test the Sleep Lock for applying critical section
// Slave program: acquire, release then increment test to declare finishing
#include <inc/lib.h>

void
_main(void)
{
	int envID = sys_getenvid();

	//Acquire the lock
	sys_utilities("__AcquireSleepLock__", 0);
	{
		if (gettst() > 1)
		{
			//Other slaves: wait for a while
			env_sleep(RAND(1000, 5000));
		}
		else
		{
			//this is the first slave inside C.S.! so wait until receiving signal from master
			while (gettst() != 1);
		}

		//Check lock value inside C.S.
		int lockVal = 0;
		sys_utilities("__GetLockValue__", (int)(&lockVal));
		if (lockVal != 1)
		{
			panic("%~test sleeplock failed! lock is not held while it's expected to be");
		}
	}
	//Release the lock
	sys_utilities("__ReleaseSleepLock__", 0);

	//indicates wakenup
	inctst();

	return;
}
