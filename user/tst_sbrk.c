
#include <inc/lib.h>
#include <user/tst_utilities.h>
//#include <kern/disk/pagefile_manager.h>

#define Mega  (1024*1024)
#define kilo (1024)

int inRange(int val, int min, int max)
{
	return (val >= min && val <= max) ? 1 : 0;
}

void _main()
{
	int i, freeFrames, freeDiskFrames;
	char *ptr;
	int eval = 0;
	bool correct = 1;
	uint32 oldBrk, newBrk;

	uint32 numOfCases = 5;
	uint32 incNumOfPages[] = {0, 1, 2, 11, (32*Mega)/PAGE_SIZE - 13};
	uint32 expectedVAs[] = {
			USER_HEAP_START, // 0
			USER_HEAP_START, // 1
			USER_HEAP_START + 0x1000, // 2
			USER_HEAP_START + 0x3000, // 11
			-1, 				// exceed (RETURN -1)
	};
	uint32 expectedSbrks[] = {
			USER_HEAP_START, // 0
			USER_HEAP_START + 0x1000, // 1
			USER_HEAP_START + 0x3000, // 2
			USER_HEAP_START + 0xe000, // 11
			USER_HEAP_START + 0xe000, // exceed
	};

	cprintf("\nSTEP A: checking sbrk() increment with zero & +ve values [80%]\n\n");
	{
		//cprintf("THE WHOLE BLOCK ALLOCATOR IS %d\n",(myEnv->uh_dynalloc_limit-USER_HEAP_START)/PAGE_SIZE);
		for (int i = 0; i < numOfCases - 1; ++i)
		{
			freeFrames = (int)sys_calculate_free_frames();
			freeDiskFrames = (int)sys_pf_calculate_allocated_pages();
			oldBrk = (uint32)sbrk(0);
			char* VA = sbrk(incNumOfPages[i]);
			newBrk = (uint32)sbrk(0);
			correct = 1;
			char c='a';
			if (((int)sys_pf_calculate_allocated_pages() - freeDiskFrames) != 0)
			{
				correct = 0;
				cprintf("A.%d: Page file is changed while it's not expected to. (pages are wrongly allocated/de-allocated in PageFile)\n", i);
			}
			if (!inRange((freeFrames - (int)sys_calculate_free_frames()),0,1))
			{
				correct = 0;
				cprintf("A.%d: Wrong memory allocation\n", i);
			}
			if ((uint32)VA != expectedVAs[i])
			{
				correct = 0;
				cprintf("A.%d: Wrong returned break: Expected: %x, Actual: %x\n", i, expectedVAs[i], VA);
			}
			if (newBrk != expectedSbrks[i])
			{
				correct = 0;
				cprintf("A.%d: Wrong new break: Expected: %x, Actual: %x\n", i, expectedSbrks[i], newBrk);
			}
			//TO CHECK THE MARKING OF THE PAGES
			if (i==2)
			{
				for(int j=0;j<2;j++)
				{
					*VA=c;
					c++;
					VA+=PAGE_SIZE;
				}
				for(int j=0;j<2;j++)
				{
					VA-=PAGE_SIZE;
					c--;
					if(*VA != c)
					{
						correct = 0;
						cprintf("Wrong values\n");
					}
				}
			}


			if (correct)
				eval += 20;
		}
	}
	cprintf("\nSTEP B: checking sbrk() increment with LARGE +ve value (EXCEED LIMIT) [20%]\n\n");
	{
		for (int i = numOfCases - 1; i < numOfCases ; ++i)
		{
			freeFrames = (int)sys_calculate_free_frames();
			freeDiskFrames = (int)sys_pf_calculate_allocated_pages();
			oldBrk = (uint32)sbrk(0);
			void* VA = sbrk(incNumOfPages[i]);
			newBrk = (uint32)sbrk(0);
			correct = 1;
			if (((int)sys_pf_calculate_allocated_pages() - freeDiskFrames) != 0)
			{
				correct = 0;
				cprintf("B.%d: Page file is changed while it's not expected to. (pages are wrongly allocated/de-allocated in PageFile)\n", i);
			}
			if ((freeFrames - (int)sys_calculate_free_frames()) != 0)
			{
				correct = 0;
				cprintf("B.%d: Wrong memory allocation\n", i);
			}
			if ((uint32)VA != expectedVAs[i])
			{
				correct = 0;
				cprintf("B.%d: Wrong returned break: Expected: %x, Actual: %x\n", i, expectedVAs[i], VA);
			}
			if (newBrk != expectedSbrks[i])
			{
				correct = 0;
				cprintf("B.%d: Wrong new break: Expected: %x, Actual: %x\n", i, expectedSbrks[i], newBrk);
			}
			if (correct)
				eval += 20;
		}
	}

	cprintf("\nTest sys_sbrk completed. Eval = %d%%\n\n", eval);

	cprintf("=================\n\n");
}
