// Scenario that tests the usage of shared variables
#include <inc/lib.h>

void InitializeAscending(int *Elements, int NumOfElements);
void InitializeDescending(int *Elements, int NumOfElements);
void InitializeSemiRandom(int *Elements, int NumOfElements);
uint32 CheckSorted(int *Elements, int NumOfElements);
void ArrayStats(int *Elements, int NumOfElements, int64 *mean, int64 *var);

void
_main(void)
{
	/*[0] GET SEMAPHORES FROM MASTER*/
	int32 parentenvID = sys_getparentenvid();
	struct semaphore arropsFinished = get_semaphore(parentenvID, "arropsFinished");
	struct semaphore cons_mutex = get_semaphore(parentenvID, "Console Mutex");

	/*[1] CREATE SEMAPHORES*/
	struct semaphore ready = create_semaphore("Ready", 0);
	struct semaphore finished = create_semaphore("Finished", 0);

	/*[2] RUN THE SLAVES PROGRAMS*/
	int numOfSlaveProgs = 3 ;

	int32 envIdQuickSort, envIdMergeSort , envIdStats ;

	//CRITICAL SECTION TO AVOID CONCURRENCY ISSUE BET MULTIPLE INTANCES OF THIS PROC IN env_create().allocate_environment()
	sys_lock_cons();
	{
		envIdQuickSort = sys_create_env("slave_qs", (myEnv->page_WS_max_size),(myEnv->SecondListSize) ,(myEnv->percentage_of_WS_pages_to_be_removed));
		envIdMergeSort = sys_create_env("slave_ms_static", (myEnv->page_WS_max_size),(myEnv->SecondListSize), (myEnv->percentage_of_WS_pages_to_be_removed));
		envIdStats = sys_create_env("slave_stats", (myEnv->page_WS_max_size), (myEnv->SecondListSize),(myEnv->percentage_of_WS_pages_to_be_removed));
	}
	sys_unlock_cons();

	if (envIdQuickSort == E_ENV_CREATION_ERROR || envIdMergeSort == E_ENV_CREATION_ERROR || envIdStats == E_ENV_CREATION_ERROR)
		panic("NO AVAILABLE ENVs...");

	sys_run_env(envIdQuickSort);
	sys_run_env(envIdMergeSort);
	sys_run_env(envIdStats);

	/*[3] CREATE SHARED VARIABLES*/
	//Share the cons_mutex owner ID
	int *mutexOwnerID = smalloc("cons_mutex ownerID", sizeof(int) , 0) ;
	*mutexOwnerID = parentenvID ;

	int ret;
	char Chose;
	char Line[30];
	int NumOfElements = RANDU(20000, 50000) ;
	int *Elements = NULL;
	//lock the console
	wait_semaphore(cons_mutex);
	{
		cprintf("\n");
		cprintf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
		cprintf("!!!   ARRAY OOERATIONS   !!!\n");
		cprintf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
		cprintf("\n");

		cprintf("Enter the number of elements: %d\n", NumOfElements);

		cprintf("Chose the initialization method:\n") ;
		cprintf("a) Ascending\n") ;
		cprintf("b) Descending\n") ;
		cprintf("c) Semi random\n");
		do
		{
			cprintf("Select: ") ;
			Chose = RANDU('a', 'd');
			cputchar(Chose);
			cputchar('\n');
		} while (Chose != 'a' && Chose != 'b' && Chose != 'c');

	}
	signal_semaphore(cons_mutex);
	//unlock the console

	//Create the shared array & its size
	int *arrSize = smalloc("arrSize", sizeof(int) , 0) ;
	*arrSize = NumOfElements ;
	Elements = smalloc("arr", sizeof(int) * NumOfElements , 0) ;

	int  i ;
	switch (Chose)
	{
	case 'a':
		InitializeAscending(Elements, NumOfElements);
		break ;
	case 'b':
		InitializeDescending(Elements, NumOfElements);
		break ;
	case 'c':
		InitializeSemiRandom(Elements, NumOfElements);
		break ;
	default:
		InitializeSemiRandom(Elements, NumOfElements);
	}

	/*[4] SIGNAL READY TO THE SLAVES*/
	for (int i = 0; i < numOfSlaveProgs; ++i) {
		signal_semaphore(ready);
	}

	/*[5] WAIT TILL ALL SLAVES FINISHED*/
	for (int i = 0; i < numOfSlaveProgs; ++i) {
		wait_semaphore(finished);
	}

	/*[6] GET THEIR RESULTS*/
	int *quicksortedArr = NULL;
	int *mergesortedArr = NULL;
	int64 *mean = NULL;
	int64 *var = NULL;
	int *min = NULL;
	int *max = NULL;
	int *med = NULL;
	quicksortedArr = sget(envIdQuickSort, "quicksortedArr") ;
	mergesortedArr = sget(envIdMergeSort, "mergesortedArr") ;
	mean = (int64*)sget(envIdStats, "mean") ;
	var = (int64*) sget(envIdStats,"var") ;
	min = sget(envIdStats,"min") ;
	max = sget(envIdStats,"max") ;
	med = sget(envIdStats,"med") ;

	/*[7] VALIDATE THE RESULTS*/
	uint32 sorted = CheckSorted(quicksortedArr, NumOfElements);
	if(sorted == 0) panic("The array is NOT quick-sorted correctly") ;
	sorted = CheckSorted(mergesortedArr, NumOfElements);
	if(sorted == 0) panic("The array is NOT merge-sorted correctly") ;
	int64 correctMean, correctVar ;
	ArrayStats(Elements, NumOfElements, &correctMean , &correctVar);
	int correctMin = quicksortedArr[0];
	int last = NumOfElements-1;
	int middle = (NumOfElements+1)/2 - 1; /*-1 to make it ZERO-Based*/
//	if (NumOfElements % 2 != 0)
//		middle--;
	int correctMax = quicksortedArr[last];
	int correctMed = quicksortedArr[middle];
	wait_semaphore(cons_mutex);
	{
		//cprintf("Array is correctly sorted\n");
		cprintf("mean = %lld, var = %lld, min = %d, max = %d, med = %d\n", *mean, *var, *min, *max, *med);
		cprintf("mean = %lld, var = %lld, min = %d, max = %d, med = %d\n", correctMean, correctVar, correctMin, correctMax, correctMed);
	}
	signal_semaphore(cons_mutex);
	if(*mean != correctMean || *var != correctVar|| *min != correctMin || *max != correctMax || *med != correctMed)
		panic("The array STATS are NOT calculated correctly") ;

	//cprintf("Congratulations!! Scenario of Using the Semaphores & Shared Variables completed successfully!!\n\n\n");
	/*[8] CHECK SEMAPHORE VALUES*/
	assert(semaphore_count(ready) == 0);
	assert(semaphore_count(finished) == 0);

	/*[9] REPORT a successful end to the master...*/
	signal_semaphore(arropsFinished);
	return;
}


uint32 CheckSorted(int *Elements, int NumOfElements)
{
	uint32 Sorted = 1 ;
	int i ;
	for (i = 0 ; i < NumOfElements - 1; i++)
	{
		if (Elements[i] > Elements[i+1])
		{
			Sorted = 0 ;
			break;
		}
	}
	return Sorted ;
}

void InitializeAscending(int *Elements, int NumOfElements)
{
	int i ;
	for (i = 0 ; i < NumOfElements ; i++)
	{
		(Elements)[i] = i ;
	}

}

void InitializeDescending(int *Elements, int NumOfElements)
{
	int i ;
	for (i = 0 ; i < NumOfElements ; i++)
	{
		Elements[i] = NumOfElements - i - 1 ;
	}

}

void InitializeSemiRandom(int *Elements, int NumOfElements)
{
	int i ;
	int Repetition = NumOfElements / 3 ;
	for (i = 0 ; i < NumOfElements ; i++)
	{
		Elements[i] = i % Repetition ;
		//cprintf("Elements[%d] = %d\n",i, Elements[i]);
	}

}

void ArrayStats(int *Elements, int NumOfElements, int64 *mean, int64 *var)
{
	int i ;
	*mean =0 ;
	for (i = 0 ; i < NumOfElements ; i++)
	{
		*mean += Elements[i];
	}
	*mean /= NumOfElements;
	*var = 0;
	for (i = 0 ; i < NumOfElements ; i++)
	{
		*var += (int64) ((Elements[i] - *mean)*(Elements[i] - *mean));
//		if (i%1000 == 0)
//			cprintf("current #elements = %d, current var = %lld\n", i , *var);
	}
	*var /= NumOfElements;
}
