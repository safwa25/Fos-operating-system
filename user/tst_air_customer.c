// Air reservation
// Customer program
#include <inc/lib.h>
#include <user/air.h>

extern volatile bool printStats;
void
_main(void)
{
	//disable the print of prog stats after finishing
	printStats = 0;

	int32 parentenvID = sys_getparentenvid();

	char _agentCapacity[] = "agentCapacity";
	char _customers[] = "customers";
	char _custCounter[] = "custCounter";
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

	char _taircl[] = "taircl";
	char _taircu[] = "taircu";

	char _flight1Customers[] = "flight1Customers";
	char _flight2Customers[] = "flight2Customers";
	char _flight3Customers[] = "flight3Customers";

	// Get the shared variables from the main program ***********************************

	struct Customer * customers = sget(parentenvID, _customers);

	int* custCounter = sget(parentenvID, _custCounter);

	int* cust_ready_queue = sget(parentenvID, _cust_ready_queue);

	int* queue_in = sget(parentenvID, _queue_in);

	int* flight1Customers = sget(parentenvID, _flight1Customers);
	int* flight2Customers = sget(parentenvID, _flight2Customers);
	int* flight3Customers = sget(parentenvID, _flight3Customers);

	// Get the shared semaphores from the main program ***********************************

	struct semaphore capacity = get_semaphore(parentenvID, _agentCapacity);
	struct semaphore custCounterCS = get_semaphore(parentenvID, _custCounterCS);
	struct semaphore clerk = get_semaphore(parentenvID, _clerk);
	struct semaphore custQueueCS = get_semaphore(parentenvID, _custQueueCS);
	struct semaphore cust_ready = get_semaphore(parentenvID, _cust_ready);
	struct semaphore custTerminated = get_semaphore(parentenvID, _custTerminated);

	int custId, flightType;
	wait_semaphore(custCounterCS);
	{
		custId = *custCounter;
		//cprintf("custCounter= %d\n", *custCounter);
		*custCounter = *custCounter +1;
		repFlightSel:
		//get random flight
		flightType = RANDU(1, 4);
		if(flightType == 1 && *flight1Customers > 0)		(*flight1Customers)--;
		else if(flightType == 2 && *flight2Customers > 0)	(*flight2Customers)--;
		else if(flightType == 3 && *flight3Customers > 0)	(*flight3Customers)--;
		else goto repFlightSel;
	}
	signal_semaphore(custCounterCS);

	//delay for a random time
	env_sleep(RANDU(100, 10000));

	//enter the agent if there's a space
	wait_semaphore(capacity);
	{
		//wait on one of the clerks
		wait_semaphore(clerk);

		//enqueue the request
		customers[custId].booked = 0 ;
		customers[custId].flightType = flightType ;
		wait_semaphore(custQueueCS);
		{
			cust_ready_queue[*queue_in] = custId;
			*queue_in = *queue_in +1;
		}
		signal_semaphore(custQueueCS);

		//signal ready
		signal_semaphore(cust_ready);

		//wait on finished
		char prefix[30]="cust_finished";
		char id[5]; char sname[50];
		ltostr(custId, id);
		strcconcat(prefix, id, sname);
		//sys_waitSemaphore(parentenvID, sname);
		struct semaphore cust_finished = get_semaphore(parentenvID, sname);
		wait_semaphore(cust_finished);

		//print the customer status
		if(customers[custId].booked == 1)
		{
			cprintf("cust %d: finished (BOOKED flight %d) \n", custId, flightType);
		}
		else
		{
			cprintf("cust %d: finished (NOT BOOKED) \n", custId);
		}
	}
	//exit the agent
	signal_semaphore(capacity);

	//customer is terminated
	signal_semaphore(custTerminated);

	return;
}
