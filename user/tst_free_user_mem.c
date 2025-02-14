/*
 * tst_free_user_mem.c
 *
 *  Created on: Dec 25, 2023
 *      Author: Mohamed Raafat
 */
/* ***********************************************************
 * > run tfum 50000
 * *********************************************************** */
#include <inc/lib.h>

#define M32 0xffffffff
struct uint64 baseTime;
//int largeGlobalSpace[10000*PAGE_SIZE];
char *itoa(uint32 value, char *result, int base);
struct uint64 subtractUint64(const struct uint64 num1, const struct uint64 num2);
void addBigNumbers(const char *num1, const char *num2, char *result);
void subtractBigNumbers(const char *num1, const char *num2, char *result);
void multiplyBigNumbers(const char *num1, const char *num2, char *result);
void divideBigNumbers(const char *number, uint32 divisor, char *result);

struct uint64 calc_execution_time(struct uint64 baseTime);
void fill_range(void *start_address, uint32 numOfFrames);

void convert_uint64_to_char(struct uint64 number, char *result);
struct uint64 convert_char_to_uint64(const char *bigNumber);

void _main(void)
{
//	largeGlobalSpace[1] = 10;

	sys_set_uheap_strategy(UHP_PLACE_FIRSTFIT);
	/*********************** NOTE ****************************
	 * WE COMPARE THE DIFF IN FREE FRAMES BY "AT LEAST" RULE
	 * INSTEAD OF "EQUAL" RULE SINCE IT'S POSSIBLE THAT SOME
	 * PAGES ARE ALLOCATED IN DYNAMIC ALLOCATOR DUE TO sbrk()
	 * (e.g. DURING THE DYNAMIC CREATION OF WS ELEMENT in FH).
	 *********************************************************/
	// Initial test to ensure it works on "PLACEMENT" not "REPLACEMENT"
	{
		if (LIST_SIZE(&(myEnv->page_WS_list)) >= myEnv->page_WS_max_size)
			panic("Please increase the WS size");
	}
	/*=================================================*/

	uint32 pagealloc_start = USER_HEAP_START + DYN_ALLOC_MAX_SIZE + PAGE_SIZE; // UHS + 32MB + 4KB

	int Mega = 1024 * 1024, kilo = 1024;
	void *ptr_allocations[10] = {0};
	uint32 allocated_frames[10] = {0};
	int idx ;
	int freeFrames, usedDiskPages, arraySizeInMB, actualNumOfFrames, expectedNumOfFrames;
	int sizeOf1stAlloc = 250*Mega;

	//[1] Allocate set of blocks
	{
		cprintf("\n[*] Part 1: Test Pre-Processing Started... \n\tAllocating Dummy LARGE Blocks --> ");
		// Allocate 250 MB
		usedDiskPages = sys_pf_calculate_allocated_pages();
		arraySizeInMB = sizeOf1stAlloc/Mega;
		ptr_allocations[5] = malloc(arraySizeInMB * Mega - kilo);
		//actualNumOfFrames = (freeFrames - sys_calculate_free_frames());
		if ((uint32)ptr_allocations[5] != (pagealloc_start))
			panic("Wrong start address for the allocated space... (Expected: %x, Received: %x)", (pagealloc_start), ptr_allocations[5]);
//		if (actualNumOfFrames < (arraySizeInMB / 4))
//			panic("Wrong fault handler: pages are not loaded successfully into memory/WS. Expected diff in frames at least = %d, actual = %d\n", (arraySizeInMB / 4), actualNumOfFrames);
		if ((sys_pf_calculate_allocated_pages() - usedDiskPages) != 0)
			panic("Wrong page file allocation: Expected no pages to be allocated.");
		freeFrames = sys_calculate_free_frames();
		idx = 5;
		allocated_frames[idx] = sizeOf1stAlloc/PAGE_SIZE;
		fill_range(ptr_allocations[idx], allocated_frames[idx]);
		actualNumOfFrames = (freeFrames - sys_calculate_free_frames()) ;
		if (actualNumOfFrames < allocated_frames[idx])
			panic("wrong number of allocated frames for ptr_allocations[%d]", idx);

		cprintf("\b\b\b\b\b\b\b\b\b\b\b\b40%%");

		// Allocate 35 MB
		usedDiskPages = sys_pf_calculate_allocated_pages();
		arraySizeInMB = 35;
		ptr_allocations[7] = malloc(arraySizeInMB * Mega - kilo);
		actualNumOfFrames = (freeFrames - sys_calculate_free_frames());
		if ((uint32)ptr_allocations[7] != (pagealloc_start + sizeOf1stAlloc))
			panic("Wrong start address for the allocated space... (Expected: %x, Received: %x)", (pagealloc_start + 25 * Mega), ptr_allocations[7]);
//		if (actualNumOfFrames < (arraySizeInMB / 4))
//			panic("Wrong fault handler: pages are not loaded successfully into memory/WS. Expected diff in frames at least = %d, actual = %d\n", (arraySizeInMB / 4), actualNumOfFrames);
		if ((sys_pf_calculate_allocated_pages() - usedDiskPages) != 0)
			panic("Wrong page file allocation: Expected no pages to be allocated.");

		freeFrames = sys_calculate_free_frames();
		idx = 7;
		allocated_frames[idx] = 8960 - 1;
		fill_range(ptr_allocations[idx], allocated_frames[idx]);
		actualNumOfFrames = (freeFrames - sys_calculate_free_frames()) ;
		if (actualNumOfFrames < allocated_frames[idx])
			panic("wrong number of allocated frames for ptr_allocations[%d]", idx);
		cprintf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b50%%");

		// Allocate 15 MB
		usedDiskPages = sys_pf_calculate_allocated_pages();
		arraySizeInMB = 15;
		ptr_allocations[8] = malloc(arraySizeInMB * Mega - kilo);
		actualNumOfFrames = (freeFrames - sys_calculate_free_frames());
		if ((uint32)ptr_allocations[8] != (pagealloc_start + sizeOf1stAlloc + 35 * Mega))
			panic("Wrong start address for the allocated space... (Expected: %x, Received: %x)", (pagealloc_start + 60 * Mega), ptr_allocations[8]);
//		if (actualNumOfFrames < (arraySizeInMB / 4))
//			panic("Wrong fault handler: pages are not loaded successfully into memory/WS. Expected diff in frames at least = %d, actual = %d\n", (arraySizeInMB / 4), actualNumOfFrames);
		if ((sys_pf_calculate_allocated_pages() - usedDiskPages) != 0)
			panic("Wrong page file allocation: Expected no pages to be allocated.");
		freeFrames = sys_calculate_free_frames();
		idx = 8;
		allocated_frames[idx] = 3840 - 1;
		fill_range(ptr_allocations[idx], allocated_frames[idx]);
		actualNumOfFrames = (freeFrames - sys_calculate_free_frames()) ;
		if (actualNumOfFrames < allocated_frames[idx])
			panic("wrong number of allocated frames for ptr_allocations[%d]", idx);
		cprintf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b55%%");

		// Allocate 25 MB
		usedDiskPages = sys_pf_calculate_allocated_pages();
		arraySizeInMB = 25;
		ptr_allocations[6] = malloc(arraySizeInMB * Mega - kilo);
		actualNumOfFrames = (freeFrames - sys_calculate_free_frames());
		if ((uint32)ptr_allocations[6] != (pagealloc_start + sizeOf1stAlloc + 50 * Mega))
			panic("Wrong start address for the allocated space... (Expected: %x, Received: %x)", (pagealloc_start + 110 * Mega), ptr_allocations[6]);
//		if (actualNumOfFrames < (arraySizeInMB / 4))
//			panic("Wrong fault handler: pages are not loaded successfully into memory/WS. Expected diff in frames at least = %d, actual = %d\n", (arraySizeInMB / 4), actualNumOfFrames);
		if ((sys_pf_calculate_allocated_pages() - usedDiskPages) != 0)
			panic("Wrong page file allocation: Expected no pages to be allocated.");
		freeFrames = sys_calculate_free_frames();
		idx = 6;
		allocated_frames[idx] = 6400 - 1;
		fill_range(ptr_allocations[idx], allocated_frames[idx]);
		actualNumOfFrames = (freeFrames - sys_calculate_free_frames()) ;
		if (actualNumOfFrames < allocated_frames[idx])
			panic("wrong number of allocated frames for ptr_allocations[%d]", idx);
		cprintf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b65%%");

		// Allocate 20 MB
		usedDiskPages = sys_pf_calculate_allocated_pages();
		arraySizeInMB = 20;
		ptr_allocations[4] = malloc(arraySizeInMB * Mega - kilo);
		actualNumOfFrames = (freeFrames - sys_calculate_free_frames());
		if ((uint32)ptr_allocations[4] != (pagealloc_start + sizeOf1stAlloc + 75 * Mega))
			panic("Wrong start address for the allocated space... (Expected: %x, Received: %x)", (pagealloc_start + 153 * Mega), ptr_allocations[4]);
//		if (actualNumOfFrames < (arraySizeInMB / 4))
//			panic("Wrong fault handler: pages are not loaded successfully into memory/WS. Expected diff in frames at least = %d, actual = %d\n", (arraySizeInMB / 4), actualNumOfFrames);
		if ((sys_pf_calculate_allocated_pages() - usedDiskPages) != 0)
			panic("Wrong page file allocation: Expected no pages to be allocated.");
		freeFrames = sys_calculate_free_frames();
		idx = 4;
		allocated_frames[idx] = 5120 - 1;
		fill_range(ptr_allocations[idx], allocated_frames[idx]);
		actualNumOfFrames = (freeFrames - sys_calculate_free_frames()) ;
		if (actualNumOfFrames < allocated_frames[idx])
			panic("wrong number of allocated frames for ptr_allocations[%d]", idx);
		cprintf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b75%%");

		// Allocate 18 MB
		usedDiskPages = sys_pf_calculate_allocated_pages();
		arraySizeInMB = 18;
		ptr_allocations[3] = malloc(arraySizeInMB * Mega - kilo);
		actualNumOfFrames = (freeFrames - sys_calculate_free_frames());
		if ((uint32)ptr_allocations[3] != (pagealloc_start + sizeOf1stAlloc + 95 * Mega))
			panic("Wrong start address for the allocated space... (Expected: %x, Received: %x)", (pagealloc_start + 135 * Mega), ptr_allocations[3]);
//		if (actualNumOfFrames < (arraySizeInMB / 4))
//			panic("Wrong fault handler: pages are not loaded successfully into memory/WS. Expected diff in frames at least = %d, actual = %d\n", (arraySizeInMB / 4), actualNumOfFrames);
		if ((sys_pf_calculate_allocated_pages() - usedDiskPages) != 0)
			panic("Wrong page file allocation: Expected no pages to be allocated.");
		freeFrames = sys_calculate_free_frames();
		idx = 3;
		allocated_frames[idx] = 4608 - 1;
		fill_range(ptr_allocations[idx], allocated_frames[idx]);
		actualNumOfFrames = (freeFrames - sys_calculate_free_frames()) ;
		if (actualNumOfFrames < allocated_frames[idx])
			panic("wrong number of allocated frames for ptr_allocations[%d]", idx);
		cprintf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b82%%");

		// Allocate 22 MB
		usedDiskPages = sys_pf_calculate_allocated_pages();
		arraySizeInMB = 22;
		ptr_allocations[2] = malloc(arraySizeInMB * Mega - kilo);
		actualNumOfFrames = (freeFrames - sys_calculate_free_frames());
		if ((uint32)ptr_allocations[2] != (pagealloc_start + sizeOf1stAlloc + 113 * Mega))
			panic("Wrong start address for the allocated space... (Expected: %x, Received: %x)", (pagealloc_start + 75 * Mega), ptr_allocations[2]);
//		if (actualNumOfFrames < (arraySizeInMB / 4))
//			panic("Wrong fault handler: pages are not loaded successfully into memory/WS. Expected diff in frames at least = %d, actual = %d\n", (arraySizeInMB / 4), actualNumOfFrames);
		if ((sys_pf_calculate_allocated_pages() - usedDiskPages) != 0)
			panic("Wrong page file allocation: Expected no pages to be allocated.");
		freeFrames = sys_calculate_free_frames();
		idx = 2;
		allocated_frames[idx] = 5632 - 1;
		fill_range(ptr_allocations[idx], allocated_frames[idx]);
		actualNumOfFrames = (freeFrames - sys_calculate_free_frames()) ;
		if (actualNumOfFrames < allocated_frames[idx])
			panic("wrong number of allocated frames for ptr_allocations[%d]", idx);
		cprintf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b92%%");

		// Allocate 13 MB
		usedDiskPages = sys_pf_calculate_allocated_pages();
		arraySizeInMB = 13;
		ptr_allocations[1] = malloc(arraySizeInMB * Mega - kilo);
		actualNumOfFrames = (freeFrames - sys_calculate_free_frames());
		if ((uint32)ptr_allocations[1] != (pagealloc_start + sizeOf1stAlloc + 135 * Mega))
			panic("Wrong start address for the allocated space... (Expected: %x, Received: %x)", (pagealloc_start + 97 * Mega), ptr_allocations[1]);
//		if (actualNumOfFrames < (arraySizeInMB / 4))
//			panic("Wrong fault handler: pages are not loaded successfully into memory/WS. Expected diff in frames at least = %d, actual = %d\n", (arraySizeInMB / 4), actualNumOfFrames);
		if ((sys_pf_calculate_allocated_pages() - usedDiskPages) != 0)
			panic("Wrong page file allocation: Expected no pages to be allocated.");
		freeFrames = sys_calculate_free_frames();
		idx = 1;
		allocated_frames[idx] = 3328 - 1;
		fill_range(ptr_allocations[idx], allocated_frames[idx]);
		actualNumOfFrames = (freeFrames - sys_calculate_free_frames()) ;
		if (actualNumOfFrames < allocated_frames[idx])
			panic("wrong number of allocated frames for ptr_allocations[%d]", idx);
		cprintf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b95%%");

		// Allocate 15 MB
		usedDiskPages = sys_pf_calculate_allocated_pages();
		arraySizeInMB = 15;
		ptr_allocations[0] = malloc(arraySizeInMB * Mega - kilo);
		actualNumOfFrames = (freeFrames - sys_calculate_free_frames());
		if ((uint32)ptr_allocations[0] != (pagealloc_start + sizeOf1stAlloc + 148 * Mega))
			panic("Wrong start address for the allocated space... (Expected: %x, Received: %x)", (pagealloc_start + 173 * Mega), ptr_allocations[0]);
//		if (actualNumOfFrames < (arraySizeInMB / 4))
//			panic("Wrong fault handler: pages are not loaded successfully into memory/WS. Expected diff in frames at least = %d, actual = %d\n", (arraySizeInMB / 4), actualNumOfFrames);
		if ((sys_pf_calculate_allocated_pages() - usedDiskPages) != 0)
			panic("Wrong page file allocation: Expected no pages to be allocated.");
		freeFrames = sys_calculate_free_frames();
		idx = 0;
		allocated_frames[idx] = 3840 - 1;
		fill_range(ptr_allocations[idx], allocated_frames[idx]);
		actualNumOfFrames = (freeFrames - sys_calculate_free_frames()) ;
		if (actualNumOfFrames < allocated_frames[idx])
			panic("wrong number of allocated frames for ptr_allocations[%d]", idx);
		cprintf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b100%%\n");
	}
	// cprintf(">>>>>>>>>>>>> %d-MB\n", (USER_HEAP_MAX - pagealloc_start) / Mega);

#define numOfSmallBlocks 1000
	void *ptr_small_allocations[numOfSmallBlocks] = {0};
	uint32 smallBlockSize = 32*kilo;
	uint32 allocated_small_frames = smallBlockSize/PAGE_SIZE;
	uint32 startAddr = (pagealloc_start + sizeOf1stAlloc + 163 * Mega);
	//[2] Allocate set of SMALL blocks
	{
		cprintf("\n[*] Part 2: Test Pre-Processing Started... \n\tAllocating Dummy SMALL Blocks --> 0%%");
		int step = numOfSmallBlocks / 100;
		for (int i = 0; i < numOfSmallBlocks; ++i)
		{
			usedDiskPages = sys_pf_calculate_allocated_pages();
			ptr_small_allocations[i] = malloc(smallBlockSize);
			if ((uint32)ptr_small_allocations[i] != (startAddr + i*smallBlockSize))
				panic("Wrong start address for the allocated space #%d... (Expected: %x, Received: %x)", i, (startAddr + i*smallBlockSize), ptr_small_allocations[i]);
			if ((sys_pf_calculate_allocated_pages() - usedDiskPages) != 0)
				panic("Wrong page file allocation: Expected no pages to be allocated.");
			freeFrames = sys_calculate_free_frames();
			fill_range(ptr_small_allocations[i], allocated_small_frames);
			actualNumOfFrames = (freeFrames - sys_calculate_free_frames()) ;
			if (actualNumOfFrames < allocated_small_frames)
				panic("wrong number of allocated frames for ptr_allocations[%d]", idx);
			if ((i+1)%step == 0)
			{
				int progress = (i+1) / step;
				if (progress <= 10)
					cprintf("%~\b\b%d%%", progress);
				else
					cprintf("%~\b\b\b%d%%", progress);
			}
		}
	}
	//[3] Free SMALL allocated memory blocks
	struct uint64 start = sys_get_virtual_time();
	{
		cprintf("\n\n[*] Part 3: Freeing allocated memory SMALL blocks. Started --> 0%%");
		int step = numOfSmallBlocks / 100;
		for (int i = 0; i < numOfSmallBlocks; i++)
		{
			freeFrames = sys_calculate_free_frames();
			usedDiskPages = sys_pf_calculate_allocated_pages();
			baseTime = sys_get_virtual_time();
			//cprintf("==> Freeing ptr[%d]\n", i);
			free(ptr_small_allocations[(i)]);
			if ((usedDiskPages - sys_pf_calculate_allocated_pages()) != 0) { panic("Wrong free: Extra or less pages are removed from PageFile\n");}
			//cprintf("sys_calculate_free_frames() - freeFrames = %d, allocated_frames[i] = %d\n", sys_calculate_free_frames() - freeFrames, allocated_frames[i]);
			if ((sys_calculate_free_frames() - freeFrames) != allocated_small_frames) { panic("Wrong free: WS pages in memory and/or page tables are not freed correctly\n");}
			if ((i+1)%step == 0)
			{
				int progress = (i+1) / step;
				if (progress <= 10)
					cprintf("%~\b\b%d%%", progress);
				else
					cprintf("%~\b\b\b%d%%", progress);
			}
		}
	}
	//[4] Free LARGE allocated memory blocks
	struct uint64 execution_time[10], total_execution_time;
	{
		cprintf("\n\n[*] Part 4: Freeing allocated memory LARGE blocks. Started...\n");

		for (int i = 0; i < 5; i++)
		{
			freeFrames = sys_calculate_free_frames();
			usedDiskPages = sys_pf_calculate_allocated_pages();
			baseTime = sys_get_virtual_time();
			//cprintf("==> Freeing ptr[%d]\n", i);
			free(ptr_allocations[(i)]);
			execution_time[(i)] = calc_execution_time(baseTime);
			if ((usedDiskPages - sys_pf_calculate_allocated_pages()) != 0) { panic("Wrong free: Extra or less pages are removed from PageFile\n");}
			//cprintf("sys_calculate_free_frames() - freeFrames = %d, allocated_frames[i] = %d\n", sys_calculate_free_frames() - freeFrames, allocated_frames[i]);
			if ((sys_calculate_free_frames() - freeFrames) != allocated_frames[i] ) { panic("Wrong free: WS pages in memory and/or page tables are not freed correctly\n");}

		}
	}
	struct uint64 end = sys_get_virtual_time();
	total_execution_time = subtractUint64(end, start);
	char sub[64];
	convert_uint64_to_char(total_execution_time, sub);
	cprintf("@@@@@@@@@@@@@@@ TOTAL EXECUTION TIME: %s (%u:%u) @@@@@@@@@@@@@@@\n", sub, total_execution_time.hi, total_execution_time.low);

	if (total_execution_time.hi > 0 || total_execution_time.low > 2000000000)
		panic("The complexity of free_user_mem is not O(1)");

	//cprintf("Congratulations... test of O(1) free user mem is run successfully within the time limit\n");
	cprintf("[#MS2EVAL#]Congratulations!!... test is completed.");

	return;
}

struct uint64 calc_execution_time(struct uint64 baseTime)
{
	struct uint64 currentTime = sys_get_virtual_time();

	char baseTimeStr[64] = {0}, currentTimeStr[64] = {0}, differenceTimeStr[64] = {0}, temp[64] = {0};
	convert_uint64_to_char(currentTime, currentTimeStr);
	convert_uint64_to_char(baseTime, baseTimeStr);
	// subtract basetime from current time
	subtractBigNumbers(currentTimeStr, baseTimeStr, differenceTimeStr);
	cprintf(">>>>> StartTime: %s, EndTime: %s, Diff: %s <<<<<\n", baseTimeStr, currentTimeStr, differenceTimeStr);
	struct uint64 r = convert_char_to_uint64(differenceTimeStr);
	// cprintf(">>>>> Difference Result: %u.%u <<<<<\n", r.hi, r.low);

	// subtract basetime from current time
	// struct uint64 res = subtractUint64(currentTime, baseTime);
	// cprintf("##### cur: %u.%u - t: %u.%u = sub: %u.%u\n", currentTime.hi, currentTime.low, baseTime.hi, baseTime.low, res.hi, res.low);
	return r;
}
struct uint64 subtractUint64(const struct uint64 num1, const struct uint64 num2)
{
	struct uint64 res;
	res.low = (num1.low - num2.low) & M32;
	res.hi = (num1.hi - num2.hi - (res.low > num1.low)) & M32;
	res.low = ((num1.low - num2.low) < 0) ? (-(num1.low - num2.low)) : res.low;
	// cprintf("##### cur: %u.%u - t: %u.%u = sub: %u.%u\n", num1.hi, num1.low, num2.hi, num2.low, res.hi, res.low);
	return res;
}

void fill_range(void *start_address, uint32 numOfFrames)
{
	uint8 *ptr = (uint8 *)start_address;
	uint32 size_to_be_filled = (numOfFrames * PAGE_SIZE);
	for (uint32 i = 0; i < size_to_be_filled; i += PAGE_SIZE)
		ptr[i] = i;
}

char *itoa(uint32 value, char *result, int base)
{
	// check that the base if valid
	if (base < 2 || base > 36)
	{
		*result = '\0';
		return result;
	}

	char *ptr = result, *ptr1 = result, tmp_char;
	int tmp_value;

	do
	{
		tmp_value = value;
		value /= base;
		*ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz"[35 + (tmp_value - value * base)];
	} while (value);

	// Apply negative sign
	if (tmp_value < 0)
		*ptr++ = '-';
	*ptr-- = '\0';

	// Reverse the string
	while (ptr1 < ptr)
	{
		tmp_char = *ptr;
		*ptr-- = *ptr1;
		*ptr1++ = tmp_char;
	}
	return result;
}
void addBigNumbers(const char *num1, const char *num2, char *result)
{
	int carry = 0;
	int len1 = strlen(num1);
	int len2 = strlen(num2);

	int maxLen = len1 > len2 ? len1 : len2;

	memset(result, '\0', strlen(result));

	for (int i = 0; i < maxLen; ++i)
	{
		int digit1 = i < len1 ? num1[len1 - 1 - i] - '0' : 0;
		int digit2 = i < len2 ? num2[len2 - 1 - i] - '0' : 0;

		int sum = digit1 + digit2 + carry;
		carry = sum / 10;
		result[i] = (sum % 10) + '0';
	}

	if (carry > 0)
	{
		result[maxLen] = carry + '0';
		result[maxLen + 1] = '\0';
	}
	else
	{
		result[maxLen] = '\0';
	}

	// Reverse the result string
	int len = strlen(result);
	for (int i = 0; i < len / 2; ++i)
	{
		char temp = result[i];
		result[i] = result[len - 1 - i];
		result[len - 1 - i] = temp;
	}
}

void subtractBigNumbers(const char *num1, const char *num2, char *result)
{
	int len1 = strlen(num1);
	int len2 = strlen(num2);
	memset(result, '\0', strlen(result));

	// Ensure num1 is greater than or equal to num2
	if (len1 < len2 || (len1 == len2 && strcmp(num1, num2) < 0))
	{
		const char *temp = num1;
		num1 = num2;
		num2 = temp;
		int temp2 = len1;
		len1 = len2;
		len2 = temp2;
	}
	int resultSize = len1 + 1; // Maximum size for the result
	result[resultSize - 1] = '\0';

	int carry = 0;
	int i, j, k;

	for (i = len1 - 1, j = len2 - 1, k = resultSize - 2; i >= 0; i--, j--, k--)
	{
		int digit1 = (i >= 0) ? (num1[i] - '0') : 0;
		int digit2 = (j >= 0) ? (num2[j] - '0') : 0;
		int tempResult = digit1 - digit2 - carry;
		if (tempResult < 0)
		{
			tempResult += 10;
			carry = 1;
		}
		else
			carry = 0;
		result[k] = tempResult + '0';
	}

	// Remove leading zeros
	while (result[0] == '0' && result[1] != '\0')
	{
		memmove(result, result + 1, resultSize - 1);
	}
}
void multiplyBigNumbers(const char *num1, const char *num2, char *result)
{
	int len1 = strlen(num1);
	int len2 = strlen(num2);
	int product[64] = {0};

	for (int i = len1 - 1; i >= 0; i--)
	{
		for (int j = len2 - 1; j >= 0; j--)
		{
			int digit1 = num1[i] - '0';
			int digit2 = num2[j] - '0';
			int partialProduct = digit1 * digit2 + product[i + j + 1];

			product[i + j + 1] = partialProduct % 10;
			product[i + j] += partialProduct / 10;
		}
	}

	int resultIndex = 0;
	while (resultIndex < len1 + len2 && product[resultIndex] == 0)
	{
		resultIndex++;
	}

	memset(result, '\0', strlen(result));
	for (int i = 0; i < len1 + len2 - resultIndex; i++)
	{
		result[i] = product[resultIndex + i] + '0';
	}

	result[len1 + len2 - resultIndex] = '\0';
}
void divideBigNumbers(const char *number, uint32 divisor, char *result)
{
	// As result can be very large store it in string
	int length = strlen(number);

	memset(result, '\0', strlen(result));
	// Find prefix of number that is larger
	// than divisor.
	int idx = 0;
	int temp = number[idx] - '0';
	while (idx < (length - 1) && temp < divisor)
		temp = temp * 10 + (number[++idx] - '0');

	// Repeatedly divide divisor with temp. After
	// every division, update temp to include one
	// more digit.
	int i = 0;
	while (length > idx)
	{
		// Store result in answer i.e. temp / divisor
		result[i++] = (temp / divisor) + '0';

		// Take next digit of number
		temp = (temp % divisor) * 10 + number[++idx] - '0';
	}
}

void convert_uint64_to_char(struct uint64 number, char *result)
{
	char low[32] = {0};
	char temp[64] = {0};
	memset(result, '\0', strlen(result));

	itoa(number.low, low, 10);

	if (number.hi > 0)
	{
		itoa(number.hi, temp, 10);
		for (int i = 0; i < 32; i++)
		{
			multiplyBigNumbers(temp, "2", result);
			strcpy(temp, result);
		}
	}
	addBigNumbers(temp, low, result);
}
struct uint64 convert_char_to_uint64(const char *bigNumber)
{
	char low[32] = {0};
	char resultStr[64] = {0}, temp[64] = {0};
	struct uint64 result;
	result.low = (uint32)strtol(bigNumber, NULL, 10);
	itoa(result.low, low, 10);
	subtractBigNumbers(bigNumber, low, temp);
	for (int i = 0; i < 32; i++)
	{
		divideBigNumbers(temp, 2, resultStr);
		strcpy(temp, resultStr);
	}
	result.hi = (uint32)strtol(temp, NULL, 10);
	return result;
}
