#include <inc/lib.h>
//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//


uint32 MarkedPages [140000] = {0};
int32 sharedObjectID [140000] = {0};

//=============================================
// [1] CHANGE THE BREAK LIMIT OF THE USER HEAP:
//=============================================
/*2023*/
void* sbrk(int increment)
{
	return (void*) sys_sbrk(increment);
}

//=================================
// [2] ALLOCATE SPACE IN USER HEAP:
//=================================
void* malloc(uint32 size)
{
	//==============================================================
	//DON'T CHANGE THIS CODE========================================
	if (size == 0)
		return NULL;
	//==============================================================
	//TODO: [PROJECT'24.MS2 - #12] [3] USER HEAP [USER SIDE] - malloc()
	// Write your code here, remove the panic and write your code
	//panic("malloc() is not implemented yet...!!");

	if (size <= DYN_ALLOC_MAX_BLOCK_SIZE) {
		return alloc_block_FF(size);
	}
	uint32 hardLimit = myEnv->UHeapHardLimit;
	uint32 page_allocator_start = hardLimit + PAGE_SIZE;
	// calculate the needed number of pages and the rounded size
	uint32 alloc_size = ROUNDUP(size, PAGE_SIZE);
	uint32 num_of_pages = alloc_size / PAGE_SIZE;

	uint32 va = page_allocator_start;
	//Check if there heap can handle this size
	if (va + alloc_size >= USER_HEAP_MAX) {
		return NULL;
	}

	uint32 foundPages = 0;
	uint32 returnAddress = va;
	//search through the user heap
	while (va <= USER_HEAP_MAX) {

		if (va >= USER_HEAP_MAX)
			return NULL;
		//check if current page is marked
		if (MarkedPages[(va - page_allocator_start) / PAGE_SIZE] != 0) {
			foundPages = 0;
			va += MarkedPages[(va - page_allocator_start) / PAGE_SIZE] * PAGE_SIZE;
			returnAddress = va;
			continue;
		}
		else{//otherwise record that you found an empty page
			foundPages += 1;
			va += PAGE_SIZE;
		}
		//mark the pages if you find the required number of page
		if (foundPages == num_of_pages) {

			va = returnAddress;
			sys_allocate_user_mem(va, alloc_size);
			for (int i = 0; i < num_of_pages; i++) {
				MarkedPages[((va + i * PAGE_SIZE) - (myEnv->UHeapHardLimit + PAGE_SIZE)) / PAGE_SIZE] = num_of_pages - i;
			}
			return (void*) returnAddress;
		}
	}
	return NULL;
	//Use sys_isUHeapPlacementStrategyFIRSTFIT() and	sys_isUHeapPlacementStrategyBESTFIT()
	//to check the current strategy

}
//=================================
// [3] FREE SPACE FROM USER HEAP:
//=================================
void free(void* virtual_address)
{
	//TODO: [PROJECT'24.MS2 - #14] [3] USER HEAP [USER SIDE] - free()
	// Write your code here, remove the panic and write your code
	//panic("free() is not implemented yet...!!");

	//if it's within dynamic allocator range then call the free block instead
	if((uint32)virtual_address >= myEnv->UHeapStart && (uint32)virtual_address < myEnv->UHeapHardLimit) {
	    free_block(virtual_address);
	}else
	{
			int numOfPages = MarkedPages[((uint32)virtual_address - (myEnv->UHeapHardLimit + PAGE_SIZE)) / PAGE_SIZE];
			uint32 itr = (uint32)virtual_address;
			sys_free_user_mem((uint32)virtual_address,numOfPages*PAGE_SIZE);
			//unmarked pages
			for(int i = 0; i < numOfPages;i++)
			{
				MarkedPages[(itr - (myEnv->UHeapHardLimit + PAGE_SIZE)) / PAGE_SIZE] = 0;
				itr += PAGE_SIZE;
			}

	}

}

//=================================
// [4] ALLOCATE SHARED VARIABLE:
//=================================
void* smalloc(char *sharedVarName, uint32 size, uint8 isWritable)
{
	//==============================================================
	//DON'T CHANGE THIS CODE========================================
	if (size == 0) return NULL ;
	//==============================================================
	//TODO: [PROJECT'24.MS2 - #18] [4] SHARED MEMORY [USER SIDE] - smalloc()
	// Write your code here, remove the panic and write your code
	//panic("smalloc() is not implemented yet...!!");
	//return NULL;

	uint32 hardLimit = myEnv->UHeapHardLimit;
		uint32 page_allocator_start = hardLimit + PAGE_SIZE;
		// calculate the needed number of pages and the rounded size
		uint32 alloc_size = ROUNDUP(size, PAGE_SIZE);
		uint32 num_of_pages = alloc_size / PAGE_SIZE;

		uint32 va = page_allocator_start;
		//Check if there heap can handle this size
		if (va + alloc_size >= USER_HEAP_MAX) {
			return NULL;
		}

		uint32 foundPages = 0;
		uint32 returnAddress = va;
		//search through the user heap
		while (va <= USER_HEAP_MAX) {

			if (va >= USER_HEAP_MAX)
				return NULL;
			//check if current page is marked
			if (MarkedPages[(va - page_allocator_start) / PAGE_SIZE] != 0) {
				foundPages = 0;
				va += MarkedPages[(va - page_allocator_start) / PAGE_SIZE] * PAGE_SIZE;
				returnAddress = va;
				continue;
			}
			else{//otherwise record that you found an empty page
				foundPages += 1;
				va += PAGE_SIZE;
			}
			//mark the pages if you find the required number of page
			if (foundPages == num_of_pages) {
				va = returnAddress;
				int32 ret = sys_createSharedObject(sharedVarName,size,isWritable,(void*)returnAddress);
				if(ret == E_SHARED_MEM_EXISTS || ret == E_NO_SHARE)
				{
					return NULL;
				}
				else
				{
					sharedObjectID[((uint32)returnAddress- (myEnv->UHeapHardLimit + PAGE_SIZE))/ PAGE_SIZE] = ret;
					for(int i = 0; i < num_of_pages;i++)
					{
						MarkedPages[((va + i * PAGE_SIZE) - (myEnv->UHeapHardLimit + PAGE_SIZE)) / PAGE_SIZE]=num_of_pages - i;
					}
				}

				return (void*)returnAddress;

			}
		}
		return NULL;

}
//========================================
// [5] SHARE ON ALLOCATED SHARED VARIABLE:
//========================================

void* sget(int32 ownerEnvID, char *sharedVarName)
{
	//TODO: [PROJECT'24.MS2 - #20] [4] SHARED MEMORY [USER SIDE] - sget()
	// Write your code here, remove the panic and write your code
	//panic("sget() is not implemented yet...!!");
	//return NULL;

	uint32 size = sys_getSizeOfSharedObject(ownerEnvID,sharedVarName);
	if(size == E_SHARED_MEM_NOT_EXISTS)
		return NULL;

	uint32 hardLimit = myEnv->UHeapHardLimit;
			uint32 page_allocator_start = hardLimit + PAGE_SIZE;
			// calculate the needed number of pages and the rounded size
			uint32 alloc_size = ROUNDUP(size, PAGE_SIZE);
			uint32 num_of_pages = alloc_size / PAGE_SIZE;

			uint32 va = page_allocator_start;
			//Check if there heap can handle this size
			if (va + alloc_size >= USER_HEAP_MAX) {
				return NULL;
			}

			uint32 foundPages = 0;
			uint32 returnAddress = va;
			//search through the user heap
			while (va <= USER_HEAP_MAX) {

				if (va >= USER_HEAP_MAX)
					return NULL;
				//check if current page is marked
				if (MarkedPages[(va - page_allocator_start) / PAGE_SIZE] != 0) {
					foundPages = 0;
					va += MarkedPages[(va - page_allocator_start) / PAGE_SIZE] * PAGE_SIZE;
					returnAddress = va;
					continue;
				}
				else{//otherwise record that you found an empty page
					foundPages += 1;
					va += PAGE_SIZE;
				}
				//mark the pages if you find the required number of page
				if (foundPages == num_of_pages) {
					va = returnAddress;
					int32 ret = sys_getSharedObject(ownerEnvID,sharedVarName,(void*)returnAddress);
					if(ret == E_SHARED_MEM_NOT_EXISTS)
					{
						return NULL;
					}
					else
					{
						sharedObjectID[((uint32)returnAddress- (myEnv->UHeapHardLimit + PAGE_SIZE))/ PAGE_SIZE] = ret;
						for(int i = 0; i < num_of_pages;i++)
						{
							MarkedPages[((va + i * PAGE_SIZE) - (myEnv->UHeapHardLimit + PAGE_SIZE)) / PAGE_SIZE]=num_of_pages - i;
						}
					}

					return (void*)returnAddress;
				}
			}
			return NULL;

}


//==================================================================================//
//============================== BONUS FUNCTIONS ===================================//
//==================================================================================//

//=================================
// FREE SHARED VARIABLE:
//=================================
//	This function frees the shared variable at the given virtual_address
//	To do this, we need to switch to the kernel, free the pages AND "EMPTY" PAGE TABLES
//	from main memory then switch back to the user again.
//
//	use sys_freeSharedObject(...); which switches to the kernel mode,
//	calls freeSharedObject(...) in "shared_memory_manager.c", then switch back to the user mode here
//	the freeSharedObject() function is empty, make sure to implement it.

void sfree(void* virtual_address)
{
	//TODO: [PROJECT'24.MS2 - BONUS#4] [4] SHARED MEMORY [USER SIDE] - sfree()
	// Write your code here, remove the panic and write your code
	//panic("sfree() is not implemented yet...!!");

	if((uint32)virtual_address < (myEnv->UHeapHardLimit + PAGE_SIZE) || (uint32)virtual_address >= USER_HEAP_MAX)
	{
		panic("virtual address in sfree (Not in page allocator Range !!\n");
	}



	int numOfPages = MarkedPages[((uint32)virtual_address - (myEnv->UHeapHardLimit + PAGE_SIZE)) / PAGE_SIZE];
	uint32 itr = (uint32)virtual_address;

	int32 id = sharedObjectID[((uint32)virtual_address - (myEnv->UHeapHardLimit + PAGE_SIZE)) /PAGE_SIZE];

	int ret = sys_freeSharedObject(id,virtual_address);
	if(ret == E_SHARED_MEM_NOT_EXISTS || ret == 0)
	{
		return;
	}
	else
	{
		sharedObjectID[((uint32)virtual_address - (myEnv->UHeapHardLimit + PAGE_SIZE)) /PAGE_SIZE] = 0;

		for(int i = 0; i < numOfPages;i++)
		{
			MarkedPages[(itr - (myEnv->UHeapHardLimit + PAGE_SIZE)) / PAGE_SIZE] = 0;
			itr += PAGE_SIZE;
		}
	}

}
//=================================
// REALLOC USER SPACE:
//=================================
//	Attempts to resize the allocated space at "virtual_address" to "new_size" bytes,
//	possibly moving it in the heap.
//	If successful, returns the new virtual_address, in which case the old virtual_address must no longer be accessed.
//	On failure, returns a null pointer, and the old virtual_address remains valid.

//	A call with virtual_address = null is equivalent to malloc().
//	A call with new_size = zero is equivalent to free().

//  Hint: you may need to use the sys_move_user_mem(...)
//		which switches to the kernel mode, calls move_user_mem(...)
//		in "kern/mem/chunk_operations.c", then switch back to the user mode here
//	the move_user_mem() function is empty, make sure to implement it.
void *realloc(void *virtual_address, uint32 new_size)
{
	//[PROJECT]
	// Write your code here, remove the panic and write your code
	panic("realloc() is not implemented yet...!!");
	return NULL;

}


//==================================================================================//
//========================== MODIFICATION FUNCTIONS ================================//
//==================================================================================//

void expand(uint32 newSize)
{
	panic("Not Implemented");

}
void shrink(uint32 newSize)
{
	panic("Not Implemented");

}
void freeHeap(void* virtual_address)
{
	panic("Not Implemented");

}
