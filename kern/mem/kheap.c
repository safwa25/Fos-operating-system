#include "kheap.h"

#include <inc/memlayout.h>
#include <inc/dynamic_allocator.h>
#include "memory_manager.h"
#include <inc/environment_definitions.h>
#include <inc/assert.h>
#include <inc/string.h>



int allocated_pages[(KERNEL_HEAP_MAX - KERNEL_HEAP_START)/PAGE_SIZE];
int allocated_blocks[(KERNEL_HEAP_MAX - KERNEL_HEAP_START)/PAGE_SIZE];

struct spinlock klock;

//Initialize the dynamic allocator of kernel heap with the given start address, size & limit
//All pages in the given range should be allocated
//Remember: call the initialize_dynamic_allocator(..) to complete the initialization
//Return:
//	On success: 0
//	Otherwise (if no memory OR initial size exceed the given limit): PANIC
int initialize_kheap_dynamic_allocator(uint32 daStart, uint32 initSizeToAllocate, uint32 daLimit)
{

	    //TODO: [PROJECT'24.MS2 - #01] [1] KERNEL HEAP - initialize_kheap_dynamic_allocator
	    // Write your code here, remove the panic and write your code
	    //panic("initialize_kheap_dynamic_allocator() is not implemented yet...!!");

		//initialize the necessary variables
		struct FrameInfo *frame_ptr;
		uint32 numberofpages=ROUNDUP(initSizeToAllocate, PAGE_SIZE)/PAGE_SIZE;
		block_allocator_start=(uint32*)daStart;
		block_allocator_limit=(uint32*)daLimit;
		ptr_break=(uint32*)(daStart +initSizeToAllocate);


		if(ptr_break > block_allocator_limit)
			panic("Initial Size Exceed Limit");


		//allocate pages
		uint32 itr = daStart;
		for (int i=0; i<numberofpages; i++)
		{
			allocate_frame(&frame_ptr);
			map_frame(ptr_page_directory, frame_ptr, itr,PERM_WRITEABLE | PERM_PRESENT);
			frame_ptr->va = itr;
			itr += PAGE_SIZE;

		}
	    initialize_dynamic_allocator(daStart,ROUNDUP(initSizeToAllocate, PAGE_SIZE));
	    init_spinlock(&klock,"kernel lock");
	    return 0;
}

void* sbrk(int numOfPages)
{
	/* numOfPages > 0: move the segment break of the kernel to increase the size of its heap by the given numOfPages,
	 * 				you should allocate pages and map them into the kernel virtual address space,
	 * 				and returns the address of the previous break (i.e. the beginning of newly mapped memory).
	 * numOfPages = 0: just return the current position of the segment break
	 *
	 * NOTES:
	 * 	1) Allocating additional pages for a kernel dynamic allocator will fail if the free frames are exhausted
	 * 		or the break exceed the limit of the dynamic allocator. If sbrk fails, return -1
	 */
	//MS2: COMMENT THIS LINE BEFORE START CODING==========
	//return (void*)-1 ;
	//====================================================

	//TODO: [PROJECT'24.MS2 - #02] [1] KERNEL HEAP - sbrk
	// Write your code here, remove the panic and write your code
	//panic("sbrk() is not implemented yet...!!");
    uint32 old_break= (uint32)ptr_break;

	if(numOfPages == 0)
		return (void *)ptr_break;

	//create the new break
	void* new_break = (void *)ptr_break + (numOfPages * PAGE_SIZE);

	if((uint32)new_break > (uint32)block_allocator_limit)
	{
		return (void *)-1;
	}
	//allocate and map the new pages
	uint32 current_va = (uint32)ptr_break;
	for (int i = 0; i < numOfPages; i++) {
				struct FrameInfo* framePtr;
	            allocate_frame(&framePtr);
	            map_frame(ptr_page_directory,framePtr,(uint32)current_va,PERM_WRITEABLE);
	            framePtr->va = current_va;
	            current_va+=PAGE_SIZE;
	}
	ptr_break= (uint32*)new_break;
	return (void*)old_break;
}
//TODO: [PROJECT'24.MS2 - BONUS#2] [1] KERNEL HEAP - Fast Page Allocator
void* kmalloc(unsigned int size)
{
	//TODO: [PROJECT'24.MS2 - #03] [1] KERNEL HEAP - kmalloc
	// Write your code here, remove the panic and write your code
	//panic("kmalloc() is not implemented yet...!!");

	acquire_spinlock(&klock);
		uint32 page_allocator_start=((uint32)block_allocator_limit+PAGE_SIZE); //CHANGED uint32 to uint8

		// calculate the needed number of pages and the rounded size
		uint32 alloc_size = ROUNDUP(size, PAGE_SIZE);
		uint32 num_of_pages = alloc_size/PAGE_SIZE;

		//pointer to the LAST allocated BLOCK (contagious pages)
		uint32 lastAllocatedBlock = page_allocator_start;

		if (size <= DYN_ALLOC_MAX_BLOCK_SIZE)
		{
			void* va =  alloc_block_FF(size);
			release_spinlock(&klock);
			return va;
		}

		uint32 va = page_allocator_start;
		if(va + alloc_size >= KERNEL_HEAP_MAX)
		{
			release_spinlock(&klock);
			return NULL;
		}

		uint32 foundPages = 0;
		uint32 returnAddress = va;
		while(va < KERNEL_HEAP_MAX)
		{
			if(va >= KERNEL_HEAP_MAX)
			{
				release_spinlock(&klock);
				return NULL;
			}
			//if the iterator encounters an allocated BLOCK (contagious pages) then it skips all allocated pages
			if(allocated_blocks[(va - page_allocator_start) / PAGE_SIZE] != 0){
				//move pointer to current allocated BLOCK (contagious pages)
				lastAllocatedBlock = va;
				foundPages = 0;
				//jump ahead the allocated pages
				va += allocated_blocks[(va - page_allocator_start) / PAGE_SIZE] * PAGE_SIZE;
				returnAddress = va;
				continue;
			}
			else
			{
				//if a free page was found, keep track of it
				foundPages += 1;
				va += PAGE_SIZE;
			}
			//if the pages found match the number of pages we need then we allocate
			if(foundPages == num_of_pages)
			{
				va = returnAddress;

				//we add the number of pages we're about to allocate to the last allocated BLOCK (contagious pages) we found
				allocated_blocks[(lastAllocatedBlock - page_allocator_start) / PAGE_SIZE] += num_of_pages;

				//we allocate and map the new frames
				struct FrameInfo * frame_ptr;
				for (int i = 0; i<(num_of_pages); i++)
				{
					allocate_frame(&frame_ptr);
					map_frame(ptr_page_directory, frame_ptr, va, PERM_WRITEABLE);
					frame_ptr->va = va;
					//array to keep track of each process's pages instead of a full allocated BLOCK (contagious pages)
					allocated_pages[(va - page_allocator_start) / PAGE_SIZE]= num_of_pages - i;
					va += PAGE_SIZE;
				}
				release_spinlock(&klock);
				return (void *)returnAddress;
			}
		}
		release_spinlock(&klock);
		return NULL;
}

void kfree(void* virtual_address)
{
	acquire_spinlock(&klock);

    uint32* va_casted = (uint32*)virtual_address;
    uint32* ptr_page_tableframes;
    struct FrameInfo* ptr_on_frame;
    //number of process pages to free
    int pages_to_free = allocated_pages[((uint32)va_casted - ((uint32)block_allocator_limit + PAGE_SIZE)) / PAGE_SIZE];
    if(va_casted >= block_allocator_start && va_casted < block_allocator_limit) {
            // Inside Block Area
            free_block(virtual_address);
            release_spinlock(&klock);
            return ;
    }else if(pages_to_free > 0)
    {
    	//Adjust the allocated BLOCKS (contagious pages) array to reflect the newly freed pages
    	uint32 va = (uint32) virtual_address;

    	//first we check if an allocated BLOCK (contagious pages) exists before the new freed block
    	while(va >= ((uint32)block_allocator_limit + PAGE_SIZE))
    	{
    		//allocated BLOCK (contagious pages) found
    		if(allocated_blocks[(va - ((uint32)block_allocator_limit + PAGE_SIZE)) / PAGE_SIZE] != 0)
    		{
    			//Calculate the original number of pages allocated in the BLOCK (contagious pages)
    			uint32 orignalPages = allocated_blocks[(va - ((uint32)block_allocator_limit + PAGE_SIZE)) / PAGE_SIZE];
    			//Calculate the number of pages lying between the freed BLOCK (contagious pages) and the start of the allocated BLOCK (contagious pages)
    			uint32 newPageNumber = ((uint32)virtual_address - va)/PAGE_SIZE;
    			//Calculate the difference between the pages in allocated BLOCK (contagious pages) and the pages lying between the freed block and allocated block and number of pages to free
    			uint32 diff = orignalPages - newPageNumber - pages_to_free;

    			//if the found allocated BLOCK (contagious pages) was out of the freed BLOCK (contagious pages) reach then break and don't do anything
    			if((va + orignalPages * PAGE_SIZE) < (uint32)virtual_address){
    				break;
    			//if the freed block will only take away a part of the end of the allocated BLOCK (contagious pages) then decrease the pages only
    			}else if(((va + orignalPages* PAGE_SIZE) > (uint32)virtual_address)&&((va + orignalPages* PAGE_SIZE) < ((uint32)virtual_address + pages_to_free * PAGE_SIZE))){

    				newPageNumber = ((va + orignalPages * PAGE_SIZE) - (uint32)virtual_address)/PAGE_SIZE;
    				allocated_blocks[(va - ((uint32)block_allocator_limit + PAGE_SIZE)) / PAGE_SIZE] -= newPageNumber;
    			}
    			else{
    				//split the allocated BLOCK (contagious pages)
    				allocated_blocks[(((uint32)virtual_address + pages_to_free * PAGE_SIZE) - ((uint32)block_allocator_limit + PAGE_SIZE)) / PAGE_SIZE] = diff;
    				allocated_blocks[(va - ((uint32)block_allocator_limit + PAGE_SIZE)) / PAGE_SIZE] = newPageNumber;
    			}

    			break;
    		}
    		va -= PAGE_SIZE;
    	}
    	//Second we check if an allocated BLOCK (contagious pages) exists AFTER the new freed block
    	uint32 pageCounter = 0;
		va = (uint32)virtual_address;
		while(pageCounter < pages_to_free){
			if(allocated_blocks[(va - ((uint32)block_allocator_limit + PAGE_SIZE)) / PAGE_SIZE] != 0){
				uint32 orignalPages = allocated_blocks[(va - ((uint32)block_allocator_limit + PAGE_SIZE)) / PAGE_SIZE];
				uint32 pagesToDecrease = (((uint32)virtual_address + pages_to_free* PAGE_SIZE) - va)/PAGE_SIZE;
				uint32 diff = orignalPages - pagesToDecrease;
				allocated_blocks[(va - ((uint32)block_allocator_limit + PAGE_SIZE)) / PAGE_SIZE] = 0;
				allocated_blocks[(((uint32)virtual_address + pages_to_free* PAGE_SIZE) - ((uint32)block_allocator_limit + PAGE_SIZE)) / PAGE_SIZE] = diff;
				break;
			}
			va+= PAGE_SIZE;
			pageCounter += 1;
		}

			//free and unmap frames
			for (int i = 0; i < pages_to_free; i++) {
			ptr_on_frame = get_frame_info(ptr_page_directory, (uint32)va_casted, &ptr_page_tableframes);
			if (ptr_on_frame != NULL) {
				free_frame(ptr_on_frame);
				unmap_frame(ptr_page_directory, (uint32)va_casted);
				ptr_on_frame->va = 0;
				allocated_pages[((uint32)va_casted - ((uint32)block_allocator_limit + PAGE_SIZE)) / PAGE_SIZE] = 0;
			}

			va_casted += PAGE_SIZE/4;  // Increment address to free the next page
		}
    } else {
    	release_spinlock(&klock);
        panic("Not bounded address in kfree()");
    }
    release_spinlock(&klock);
}

unsigned int kheap_physical_address(unsigned int virtual_address)
{
	if ((uint32)virtual_address < KERNEL_HEAP_START || (uint32)virtual_address > KERNEL_HEAP_MAX)
			    	return 0;

			uint32 *ptr_page_table;
			int return_table = get_page_table(ptr_page_directory , (uint32)virtual_address ,&ptr_page_table);
			if(return_table == TABLE_IN_MEMORY)
			{

		        uint32 page_table_entry = ptr_page_table[PTX((uint32)virtual_address)];
			    if ((page_table_entry & PERM_PRESENT) == PERM_PRESENT) // exist mapping
			    {
			    	uint32 frame_number = page_table_entry & 0xFFFFF000;
			        uint32 offset = (uint32)virtual_address & 0x00000FFF;
			    	unsigned int physical_address = frame_number | offset; // frame number + offset
			    	return physical_address;
			    }
			    else
			    	return 0;
			}

			return 0;
}

unsigned int kheap_virtual_address(unsigned int physical_address)
{
	//TODO: [PROJECT'24.MS2 - #06] [1] KERNEL HEAP - kheap_virtual_address
		// Write your code here, remove the panic and write your code
		//panic("kheap_virtual_address() is not implemented yet...!!");
		struct FrameInfo *curr_Frame = to_frame_info((uint32)physical_address);
		uint32 offset = (uint32)physical_address % PAGE_SIZE;
		if(curr_Frame->references == 0)
		{
			return 0;
		}
		uint32 virtual_address = curr_Frame->va +  offset;
	    if (virtual_address < KERNEL_HEAP_START || virtual_address >= KERNEL_HEAP_MAX)
	    	return 0;

		return (unsigned int)(virtual_address);


		//return the virtual address corresponding to given physical_address
		//refer to the project presentation and documentation for details

		//EFFICIENT IMPLEMENTATION ~O(1) IS REQUIRED ==================
}

//=================================================================================//
//============================== BONUS FUNCTION ===================================//
//=================================================================================//
// krealloc():

//	Attempts to resize the allocated space at "virtual_address" to "new_size" bytes,
//	possibly moving it in the heap.
//	If successful, returns the new virtual_address, if moved to another loc: the old virtual_address must no longer be accessed.
//	On failure, returns a null pointer, and the old virtual_address remains valid.

//	A call with virtual_address = null is equivalent to kmalloc().
//	A call with new_size = zero is equivalent to kfree().

void *krealloc(void *virtual_address, uint32 new_size)
{
	//TODO: [PROJECT'24.MS2 - BONUS#1] [1] KERNEL HEAP - krealloc
	// Write your code here, remove the panic and write your code
	//return NULL;
	//panic("krealloc() is not implemented yet...!!");
	uint8 convert_to_page= new_size>DYN_ALLOC_MAX_BLOCK_SIZE;
	uint8 current_VA_is_page= (virtual_address >= (void *)KERNEL_HEAP_START && virtual_address < (void *)KERNEL_HEAP_MAX);
	void* page_allocator_start= block_allocator_limit+PAGE_SIZE;
	if (new_size==0 && virtual_address !=NULL){
		kfree(virtual_address);
		return NULL;
	}
	if(new_size!=0 && virtual_address==NULL)
		return kmalloc(new_size);

	if(new_size==0 && virtual_address ==NULL)
		return NULL;

	//the new size and the current size are in the block size range (block to block)
	if(!convert_to_page&&!current_VA_is_page)
	{
		return realloc_block_FF(virtual_address,new_size);
	}
	//the the new size is block size and the current is page size (convert page to block)
	if(!convert_to_page&&current_VA_is_page)
	{
		void* created_block_VA=alloc_block_FF(new_size);
		if(created_block_VA==NULL)	return virtual_address;
		memmove(created_block_VA,virtual_address,new_size);
		kfree(virtual_address);
		return created_block_VA;
	}
	//the new size is page size and the current size is block size (convert block to page)
	if(convert_to_page&&!current_VA_is_page)
	{
		//uint32 alloc_size = ROUNDUP(new_size, PAGE_SIZE);
		void* created_pages_VA=kmalloc(new_size);
		if(created_pages_VA==NULL)	return virtual_address;
		memmove(created_pages_VA,virtual_address,new_size);
		free_block(virtual_address);
		return created_pages_VA;
	}
	//the new size and the current size are in the page size range (page to page)
	if(convert_to_page&&current_VA_is_page)
	{
		uint32 alloc_size = ROUNDUP(new_size, PAGE_SIZE);
		uint32 new_num_of_pages = alloc_size/PAGE_SIZE;
		// page # of the given virtual address
		uint32 VA_page_number=(virtual_address-((void*)block_allocator_limit+PAGE_SIZE))/PAGE_SIZE;
		// # of pages allocated in this virtual address
		uint32 current_num_of_pages = allocated_pages[VA_page_number];
		// new VA for the next page after the contiguous pages in given VA
		void* next_page_VA= virtual_address + current_num_of_pages * PAGE_SIZE;
		//difference is +ev if increase size else -ev
		int pages_difference = new_num_of_pages- current_num_of_pages;

		// if the new size(not the rounded size) == the current size return the VA
		if(current_num_of_pages*PAGE_SIZE==new_size)
			return virtual_address;

		// reallocate after increase size
		if(pages_difference>0)
		{
			uint8 can_extend = 1;
			uint32 iterator = VA_page_number + current_num_of_pages;
			uint32 check_iterator=iterator;
			for(int i=0; i<pages_difference;i++)
			{
				if(allocated_pages[check_iterator]>0)
				{
					can_extend=0;
					break;
				}
				check_iterator++;
			}
			// no more contiguous size available (free current and call kmalloc)
			if(!can_extend)
			{
				void *new_page_VA= kmalloc(new_size);
				cprintf("the VA is after the start by: %d mega",(((uint32)new_page_VA-((uint32)block_allocator_limit + PAGE_SIZE)))/(PAGE_SIZE *PAGE_SIZE/16) );
				if(new_page_VA==NULL)	return virtual_address;
				memmove(new_page_VA,virtual_address,new_size);
				kfree(virtual_address);
				return new_page_VA;
			}
			// can add more pages for the current VA ()
			else
			{
				void* map_iterator=next_page_VA;
				allocated_pages[VA_page_number]=new_num_of_pages;
				struct FrameInfo *frame_ptr;
				uint32 num_of_pages=pages_difference;
				for(int i=0; i<pages_difference;i++)
				{
					allocate_frame(&frame_ptr);
					map_frame(ptr_page_directory, frame_ptr, (uint32)map_iterator, PERM_WRITEABLE | PERM_PRESENT);
					frame_ptr->va=(uint32)map_iterator;
					allocated_pages[(map_iterator - page_allocator_start) / PAGE_SIZE]= num_of_pages--;
					map_iterator += PAGE_SIZE;
}
				return virtual_address;
			}
		}
		// reallocate decrease increase size
		else if(pages_difference<0)
		{
			allocated_pages[VA_page_number]=new_num_of_pages;
			void* free_iterator=next_page_VA-PAGE_SIZE;
			uint32* ptr_page_tableframes;
			struct FrameInfo* ptr_on_frame;
			uint32 pages_to_free= pages_difference*-1;
			for(int i =pages_to_free; i>0;i--)
			{
				ptr_on_frame = get_frame_info(ptr_page_directory, (uint32)free_iterator, &ptr_page_tableframes);
				if (ptr_on_frame != NULL) {
					free_frame(ptr_on_frame);
					unmap_frame(ptr_page_directory, (uint32)free_iterator);
					ptr_on_frame->va=0;
					allocated_pages[((uint32)free_iterator - ((uint32)block_allocator_limit + PAGE_SIZE)) / PAGE_SIZE] = 0;
				}
				free_iterator -= PAGE_SIZE;
			}

			//if the size decreased [12k-->9K] then you should remove the last 3K
			memmove(virtual_address,virtual_address,new_size);

		}
		return virtual_address;
	}
	return NULL;
}
