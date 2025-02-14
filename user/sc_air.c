// Air reservation problem
// Master program
#include <inc/lib.h>
#include <user/air.h>
int find(int* arr, int size, int val);

void
_main(void)
{
	int envID = sys_getenvid();

	int numOfClerks = 3;
	int agentCapacity = 20;
	int numOfCustomers = 50;
	int flight1NumOfCustomers = numOfCustomers/3;
	int flight2NumOfCustomers = numOfCustomers/3;
	int flight3NumOfCustomers = numOfCustomers - (flight1NumOfCustomers + flight2NumOfCustomers);

	int flight1NumOfTickets = 15;
	int flight2NumOfTickets = 15;

	// *************************************************************************************************
	/// Reading Inputs *********************************************************************************
	// *************************************************************************************************
	char Line[255] ;
	char Chose;
	sys_lock_cons();
	{
		cprintf("\n");
		cprintf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
		cprintf("!!!! AIR PLANE RESERVATION !!!!\n");
		cprintf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
		cprintf("\n");
		cprintf("%~Default #customers = %d (equally divided over the 3 flights).\n"
				"Flight1 Tickets = %d, Flight2 Tickets = %d\n"
				"Agent Capacity = %d\n", numOfCustomers, flight1NumOfTickets, flight2NumOfTickets, agentCapacity) ;
		Chose = 0 ;
		while (Chose != 'y' && Chose != 'n' && Chose != 'Y' && Chose != 'N')
		{
			cprintf("%~Do you want to change these values(y/n)? ") ;
			Chose = 'n' ;
			cputchar(Chose);
			cputchar('\n');
			cputchar('\n');
		}
		if (Chose == 'y' || Chose == 'Y')
		{
			readline("Enter the capacity of the agent: ", Line);
			agentCapacity = strtol(Line, NULL, 10) ;
			readline("Enter the total number of customers: ", Line);
			numOfCustomers = strtol(Line, NULL, 10) ;
			flight1NumOfCustomers = flight2NumOfCustomers = numOfCustomers / 3;
			flight3NumOfCustomers = numOfCustomers - (flight1NumOfCustomers + flight2NumOfCustomers);
			readline("Enter # tickets of flight#1: ", Line);
			flight1NumOfTickets = strtol(Line, NULL, 10) ;
			readline("Enter # tickets of flight#2: ", Line);
			flight2NumOfTickets = strtol(Line, NULL, 10) ;
		}
	}
	sys_unlock_cons();

	// *************************************************************************************************
	/// Shared Variables Region ************************************************************************
	// *************************************************************************************************
	char _isOpened[] = "isOpened";
	char _agentCapacity[] = "agentCapacity";
	char _customers[] = "customers";
	char _custCounter[] = "custCounter";
	char _flight1Customers[] = "flight1Customers";
	char _flight2Customers[] = "flight2Customers";
	char _flight3Customers[] = "flight3Customers";
	char _flight1Counter[] = "flight1Counter";
	char _flight2Counter[] = "flight2Counter";
	char _flightBooked1Counter[] = "flightBooked1Counter";
	char _flightBooked2Counter[] = "flightBooked2Counter";
	char _flightBooked1Arr[] = "flightBooked1Arr";
	char _flightBooked2Arr[] = "flightBooked2Arr";
	char _cust_ready_queue[] = "cust_ready_queue";
	char _queue_in[] = "queue_in";
	char _queue_out[] = "queue_out";

	char _cust_ready[] = "cust_ready";
	char _custQueueCS[] = "custQueueCS";
	char _flight1CS[] = "flight1CS";
	char _flight2CS[] = "flight2CS";

	char _clerk[] = "clerk";
	char _custCounterCS[] = "custCounterCS";
	char _custTerminated[] = "custTerminated";
	char _clerkTerminated[] = "clerkTerminated";

	char _taircl[] = "taircl";
	char _taircu[] = "taircu";

	struct Customer * custs;
	custs = smalloc(_customers, sizeof(struct Customer)*(numOfCustomers+1), 1);
	//sys_createSharedObject("customers", sizeof(struct Customer)*numOfCustomers, 1, (void**)&custs);

	int* flight1Customers = smalloc(_flight1Customers, sizeof(int), 1); *flight1Customers = flight1NumOfCustomers;
	int* flight2Customers = smalloc(_flight2Customers, sizeof(int), 1); *flight2Customers = flight2NumOfCustomers;
	int* flight3Customers = smalloc(_flight3Customers, sizeof(int), 1); *flight3Customers = flight3NumOfCustomers;

	int* isOpened = smalloc(_isOpened, sizeof(int), 0);
	*isOpened = 1;

	int* custCounter = smalloc(_custCounter, sizeof(int), 1);
	*custCounter = 0;

	int* flight1Counter = smalloc(_flight1Counter, sizeof(int), 1);
	*flight1Counter = flight1NumOfTickets;

	int* flight2Counter = smalloc(_flight2Counter, sizeof(int), 1);
	*flight2Counter = flight2NumOfTickets;

	int* flight1BookedCounter = smalloc(_flightBooked1Counter, sizeof(int), 1);
	*flight1BookedCounter = 0;

	int* flight2BookedCounter = smalloc(_flightBooked2Counter, sizeof(int), 1);
	*flight2BookedCounter = 0;

	int* flight1BookedArr = smalloc(_flightBooked1Arr, sizeof(int)*flight1NumOfTickets, 1);
	int* flight2BookedArr = smalloc(_flightBooked2Arr, sizeof(int)*flight2NumOfTickets, 1);

	int* cust_ready_queue = smalloc(_cust_ready_queue, sizeof(int)*(numOfCustomers+1), 1);

	int* queue_in = smalloc(_queue_in, sizeof(int), 1);
	*queue_in = 0;

	int* queue_out = smalloc(_queue_out, sizeof(int), 1);
	*queue_out = 0;

	// *************************************************************************************************
	/// Semaphores Region ******************************************************************************
	// *************************************************************************************************
	struct semaphore capacity = create_semaphore(_agentCapacity, agentCapacity);

	struct semaphore flight1CS = create_semaphore(_flight1CS, 1);
	struct semaphore flight2CS = create_semaphore(_flight2CS, 1);

	struct semaphore custCounterCS = create_semaphore(_custCounterCS, 1);
	struct semaphore custQueueCS = create_semaphore(_custQueueCS, 1);

	struct semaphore clerk = create_semaphore(_clerk, 3);

	struct semaphore cust_ready = create_semaphore(_cust_ready, 0);

	struct semaphore custTerminated = create_semaphore(_custTerminated, 0);
	struct semaphore clerkTerminated = create_semaphore(_clerkTerminated, 0);

	struct semaphore* cust_finished = smalloc("cust_finished_array", numOfCustomers*sizeof(struct semaphore), 1);

	int s=0;
	for(s=0; s<numOfCustomers; ++s)
	{
		char prefix[30]="cust_finished";
		char id[5]; char sname[50];
		ltostr(s, id);
		strcconcat(prefix, id, sname);
		//sys_createSemaphore(sname, 0);
		cust_finished[s] = create_semaphore(sname, 0);
	}

	// *************************************************************************************************
	// start all clerks and customers ******************************************************************
	// *************************************************************************************************

	char setPriorityCmd[100] = "__PRIRRSetPriority@";

	//clerks
	int* clerkEnvIds = malloc(numOfClerks * sizeof(int));
	int* custEnvIds = malloc(numOfCustomers * sizeof(int));

	int c, k;

	//CRITICAL SECTION TO AVOID CONCURRENCY ISSUE BET MULTIPLE INTANCES OF THIS PROC IN env_create().allocate_environment()
	sys_lock_cons();
	{
		for (k = 0; k < numOfClerks; ++k)
		{
			clerkEnvIds[k] = sys_create_env(_taircl, (myEnv->page_WS_max_size),(myEnv->SecondListSize), (myEnv->percentage_of_WS_pages_to_be_removed));
			if (clerkEnvIds[k] == E_ENV_CREATION_ERROR)
				panic("NO AVAILABLE ENVs... Please reduce the num of customers and try again");

			char id[20] ;
			ltostr(clerkEnvIds[k], id);
			char setPriorityWithIDCmd[100] ;
			strcconcat(setPriorityCmd, id, setPriorityWithIDCmd);
			sys_utilities(setPriorityWithIDCmd, k * 10);
		}

		//customers
		for(c=0; c< numOfCustomers;++c)
		{
			custEnvIds[c] = sys_create_env(_taircu, (myEnv->page_WS_max_size),(myEnv->SecondListSize), (myEnv->percentage_of_WS_pages_to_be_removed));
			if (custEnvIds[c] == E_ENV_CREATION_ERROR)
				panic("NO AVAILABLE ENVs... Please reduce the num of customers and try again");

			char id[20] ;
			ltostr(custEnvIds[c], id);
			char setPriorityWithIDCmd[100] ;
			strcconcat(setPriorityCmd, id, setPriorityWithIDCmd);
			sys_utilities(setPriorityWithIDCmd, RANDU(0, 30));
		}
	}
	sys_unlock_cons();

	//Run Clerks
	for (k = 0; k < numOfClerks; ++k)
	{
		sys_run_env(clerkEnvIds[k]);
	}
	//Run Customers
	for(c=0; c < numOfCustomers;++c)
	{
		sys_run_env(custEnvIds[c]);
	}

	//wait until all customers terminated
	for(c=0; c< numOfCustomers;++c)
	{
		wait_semaphore(custTerminated);
	}

	env_sleep(1500);
	int b;

	sys_lock_cons();
	{
		//print out the results
		for(b=0; b< (*flight1BookedCounter);++b)
		{
			cprintf("cust %d booked flight 1, originally ordered %d\n", flight1BookedArr[b], custs[flight1BookedArr[b]].flightType);
		}

		for(b=0; b< (*flight2BookedCounter);++b)
		{
			cprintf("cust %d booked flight 2, originally ordered %d\n", flight2BookedArr[b], custs[flight2BookedArr[b]].flightType);
		}
	}
	sys_unlock_cons();

	int numOfBookings = 0;
	int numOfFCusts[3] = {0};

	for(b=0; b< numOfCustomers;++b)
	{
		if (custs[b].booked)
		{
			numOfBookings++;
			numOfFCusts[custs[b].flightType - 1]++ ;
		}
	}

	sys_lock_cons();
	{
		cprintf("%~[*] FINAL RESULTS:\n");
		cprintf("%~\tTotal number of customers = %d (Flight1# = %d, Flight2# = %d, Flight3# = %d)\n", numOfCustomers, flight1NumOfCustomers,flight2NumOfCustomers,flight3NumOfCustomers);
		cprintf("%~\tTotal number of customers who receive tickets = %d (Flight1# = %d, Flight2# = %d, Flight3# = %d)\n", numOfBookings, numOfFCusts[0],numOfFCusts[1],numOfFCusts[2]);
	}
	sys_unlock_cons();

	//check out the final results and semaphores
	{
		for(int c = 0; c < numOfCustomers; ++c)
		{
			if (custs[c].booked)
			{
				if(custs[c].flightType ==1 && find(flight1BookedArr, flight1NumOfTickets, c) != 1)
				{
					panic("Error, wrong booking for user %d\n", c);
				}
				if(custs[c].flightType ==2 && find(flight2BookedArr, flight2NumOfTickets, c) != 1)
				{
					panic("Error, wrong booking for user %d\n", c);
				}
				if(custs[c].flightType ==3 && ((find(flight1BookedArr, flight1NumOfTickets, c) + find(flight2BookedArr, flight2NumOfTickets, c)) != 2))
				{
					panic("Error, wrong booking for user %d\n", c);
				}
			}
		}

		assert(semaphore_count(capacity) == agentCapacity);

		assert(semaphore_count(flight1CS) == 1);
		assert(semaphore_count(flight2CS) == 1);

		assert(semaphore_count(custCounterCS) ==  1);
		assert(semaphore_count(custQueueCS)  ==  1);

		assert(semaphore_count(clerk)  == 3);

		assert(semaphore_count(cust_ready) == -3);

		assert(semaphore_count(custTerminated) ==  0);

		int s=0;
		for(s=0; s<numOfCustomers; ++s)
		{
			assert(semaphore_count(cust_finished[s]) ==  0);
		}

		atomic_cprintf("%~\nAll reservations are successfully done... have a nice flight :)\n");

		//waste some time then close the agency
		env_sleep(5000) ;
		*isOpened = 0;
		atomic_cprintf("\n%~The agency is closing now...\n");

		//Signal all clerks to continue and recheck the isOpened flag
		cust_ready_queue[numOfCustomers] = -1; //to indicate, for the clerk, there's no more customers
		for (int k = 0; k < numOfClerks; ++k)
		{
			signal_semaphore(cust_ready);
		}

		//Wait all clerks to finished
		for (int k = 0; k < numOfClerks; ++k)
		{
			wait_semaphore(clerkTerminated);
		}

		assert(semaphore_count(clerkTerminated) ==  0);

	}

	//Delete IDs arrays
	free(clerkEnvIds);
	free(custEnvIds);

	atomic_cprintf("%~\nAirplane Reservation App is Finished Successfully\n\n");

	//INDICATE A SUCCESSFUL FINISHING
	inctst();
}


int find(int* arr, int size, int val)
{
	int result = 0;

	int i;
	for(i=0; i<size;++i )
	{
		if(arr[i] == val)
		{
			result = 1;
			break;
		}
	}

	return result;
}
