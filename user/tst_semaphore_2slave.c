// Test the use of semaphores to allow multiprograms to enter the CS at same time
// Slave program: enter the shop, leave it and signal the master program

#include <inc/lib.h>
extern volatile bool printStats;

void
_main(void)
{
	printStats = 0;
	int id = sys_getenvindex();

	int32 parentenvID = sys_getparentenvid();
	//cprintf("Cust %d: outside the shop\n", id);
	struct semaphore shopCapacitySem = get_semaphore(parentenvID, "shopCapacity");
	struct semaphore dependSem = get_semaphore(parentenvID, "depend");

	wait_semaphore(shopCapacitySem);
	{
		atomic_cprintf("Cust %d: inside the shop\n", id) ;
		env_sleep(RANDU(100,1000)) ;
	}
	signal_semaphore(shopCapacitySem);

	atomic_cprintf("Cust %d: exit the shop\n", id);
	signal_semaphore(dependSem);
	return;
}
