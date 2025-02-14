#include <inc/assert.h>
#include <inc/string.h>
#include "../inc/dynamic_allocator.h"


//==================================================================================//
//============================== GIVEN FUNCTIONS ===================================//
//==================================================================================//

//=====================================================
// 1) GET BLOCK SIZE (including size of its meta data):
//=====================================================
__inline__ uint32 get_block_size(void* va)
{
	uint32 *curBlkMetaData = ((uint32 *)va - 1) ;
	return (*curBlkMetaData) & ~(0x1);
}

//===========================
// 2) GET BLOCK STATUS:
//===========================
__inline__ int8 is_free_block(void* va)
{
	uint32 *curBlkMetaData = ((uint32 *)va - 1) ;
	return (~(*curBlkMetaData) & 0x1) ;
}

//===========================
// 3) ALLOCATE BLOCK:
//===========================

void *alloc_block(uint32 size, int ALLOC_STRATEGY)
{
	void *va = NULL;
	switch (ALLOC_STRATEGY)
	{
	case DA_FF:
		va = alloc_block_FF(size);
		break;
	case DA_NF:
		va = alloc_block_NF(size);
		break;
	case DA_BF:
		va = alloc_block_BF(size);
		break;
	case DA_WF:
		va = alloc_block_WF(size);
		break;
	default:
		cprintf("Invalid allocation strategy\n");
		break;
	}
	return va;
}

//===========================
// 4) PRINT BLOCKS LIST:
//===========================

void print_blocks_list(struct MemBlock_LIST list)
{
	cprintf("=========================================\n");
	struct BlockElement* blk ;
	cprintf("\nDynAlloc Blocks List:\n");
	LIST_FOREACH(blk, &list)
	{
		cprintf(" va :%p : (size: %d, isFree: %d)\n",blk, get_block_size(blk), is_free_block(blk)) ;
	}
	cprintf("=========================================\n");

}
//
////********************************************************************************//
////********************************************************************************//

//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//

bool is_initialized = 0;
//==================================
// [1] INITIALIZE DYNAMIC ALLOCATOR:
//==================================
void initialize_dynamic_allocator(uint32 daStart, uint32 initSizeOfAllocatedSpace)
{
	//==================================================================================
	//DON'T CHANGE THESE LINES==========================================================
	//==================================================================================
	{
		if (initSizeOfAllocatedSpace % 2 != 0) initSizeOfAllocatedSpace++; //ensure it's multiple of 2
		if (initSizeOfAllocatedSpace == 0)
			return ;
		is_initialized = 1;
	}
	//==================================================================================
	//==================================================================================

	//TODO: [PROJECT'24.MS1 - #04] [3] DYNAMIC ALLOCATOR - initialize_dynamic_allocator
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("initialize_dynamic_allocator is not implemented yet");
	//Your Code is Here...
	int *ptr = (void *) daStart;
	*ptr = 1;
	//set_block_data(ptr, initSizeOfAllocatedSpace-8, 0);
	ptr++;
	*ptr = initSizeOfAllocatedSpace - 8;
	ptr++;
	LIST_INIT(&freeBlocksList);
	struct BlockElement *blockPtr = (void *) ptr;
	LIST_INSERT_HEAD(&freeBlocksList, blockPtr);
	ptr = ptr + (initSizeOfAllocatedSpace - 16)/4;
	*ptr = initSizeOfAllocatedSpace - 8;
	ptr++;
	*ptr = 1;
}
//==================================
// [2] SET BLOCK HEADER & FOOTER:
//==================================
void set_block_data(void* va, uint32 totalSize, bool isAllocated)
{
	//TODO: [PROJECT'24.MS1 - #05] [3] DYNAMIC ALLOCATOR - set_block_data
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("set_block_data is not implemented yet");
	//Your Code is Here...
	uint32 *headerNewMetaData = ((uint32 *)va - 1) ;
		if(isAllocated)
			*headerNewMetaData=(totalSize| 0x1);
		else
			*headerNewMetaData=(totalSize& ~(0x1));//0001
		  	  	  	  	  	  	  	  	  	  	  //1110
												  //0101

		uint32 *footerNewMetaData = (uint32 *) ((uint8 *)va + totalSize - 8);
		if(isAllocated)
			*footerNewMetaData=(totalSize| 0x1);
		else
			*footerNewMetaData=(totalSize& ~(0x1));
}


//=========================================
// [3] ALLOCATE BLOCK BY FIRST FIT:
//=========================================
void *alloc_block_FF(uint32 size)
{//==================================================================================
	//DON'T CHANGE THESE LINES==========================================================
	//==================================================================================
	{
		if (size % 2 != 0) size++;	//ensure that the size is even (to use LSB as allocation flag)
		if (size < DYN_ALLOC_MIN_BLOCK_SIZE)
			size = DYN_ALLOC_MIN_BLOCK_SIZE ;
		if (!is_initialized)
		{
			uint32 required_size = size + 2*sizeof(int) /*header & footer*/ + 2*sizeof(int) /*da begin & end*/ ;
			uint32 da_start = (uint32)sbrk(ROUNDUP(required_size, PAGE_SIZE)/PAGE_SIZE);
			uint32 da_break = (uint32)sbrk(0);
			initialize_dynamic_allocator(da_start, da_break - da_start);
		}
	}
	//==================================================================================
	//==================================================================================

	//TODO: [PROJECT'24.MS1 - #06] [3] DYNAMIC ALLOCATOR - alloc_block_FF
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("alloc_block_FF is not implemented yet");
	//Your Code is Here...
	//  following code for debugging
	size = size + 8;
		struct BlockElement *block;
			int flag=0;
				LIST_FOREACH(block, &freeBlocksList) {
					if(get_block_size(block)>=size)
					{
						flag=1;
						int diff=get_block_size(block)-size;
						if(diff>=16)// if(block size is bit larger)
						{
							//Create New Allocated Block
							struct BlockElement *AllocatedBlock;
							set_block_data(block,size,1);
							AllocatedBlock = block;
							//Create New Empty Block
							struct BlockElement *new_Empty_Block = (void *)AllocatedBlock + size;
							set_block_data(new_Empty_Block,diff,0);
							//Adjust Previous Pointer
							LIST_INSERT_BEFORE(&freeBlocksList,block,new_Empty_Block);
							LIST_REMOVE(&freeBlocksList,block);

							return AllocatedBlock;

						}
						else
						{
							LIST_REMOVE(&freeBlocksList,block);
							set_block_data(block,size + diff,1);

							return block;
						}
					}
				}
				uint16 no_of_pages = ROUNDUP(size, PAGE_SIZE)/PAGE_SIZE;
				void *da_break = sbrk(no_of_pages);
				if((int)da_break ==-1) return NULL;
				if (LIST_SIZE(&freeBlocksList) != 0)
				{
					block = LIST_LAST(&freeBlocksList);
					void *ptr = (void *) block + get_block_size(block);
					if(ptr == da_break)
						{
						set_block_data(block, get_block_size(block) + no_of_pages*PAGE_SIZE, 0);
						}
					else
					{
						set_block_data(da_break,no_of_pages*PAGE_SIZE, 0);// the error in free
					struct BlockElement * new_block = da_break;
					LIST_INSERT_TAIL(&freeBlocksList, new_block);
					}

				}
				else
				{
					set_block_data(da_break, no_of_pages*PAGE_SIZE, 0);
					struct BlockElement * new_block = da_break;
					LIST_INSERT_TAIL(&freeBlocksList, new_block);
				}
				da_break+=no_of_pages*PAGE_SIZE;
				uint32 *end_block = da_break - 4;
				*end_block = 1;
				return alloc_block_FF(size - 8);
}
//=========================================
// [4] ALLOCATE BLOCK BY BEST FIT:
//=========================================
void *alloc_block_BF(uint32 size)
{

	{
			if (size % 2 != 0) size++;	//ensure that the size is even (to use LSB as allocation flag)
			if (size < DYN_ALLOC_MIN_BLOCK_SIZE)
				size = DYN_ALLOC_MIN_BLOCK_SIZE ;
			if (!is_initialized)
			{
				uint32 required_size = size + 2*sizeof(int) /*header & footer*/ + 2*sizeof(int) /*da begin & end*/ ;
				uint32 da_start = (uint32)sbrk(ROUNDUP(required_size, PAGE_SIZE)/PAGE_SIZE);
				uint32 da_break = (uint32)sbrk(0);
				initialize_dynamic_allocator(da_start, da_break - da_start);
			}
	}
		//==================================================================================
		//==================================================================================

		//TODO: [PROJECT'24.MS1 - #06] [3] DYNAMIC ALLOCATOR - alloc_block_BF
		//COMMENT THE FOLLOWING LINE BEFORE START CODING
		//panic("alloc_block_BF is not implemented yet");
		//Your Code is Here...
		//  following code for debugging
		size = size + 8;
		struct BlockElement *block;
		struct BlockElement *best_block=NULL;//=LIST_FIRST(&freeBlocksList);

		LIST_FOREACH(block, &freeBlocksList) {
			if(get_block_size(block)>=size)
			{
				if(best_block==NULL)
					best_block=block;
				if(best_block!=NULL&&get_block_size(block)<get_block_size(best_block))
					best_block=block;
			}
		}
		if(best_block!=NULL&&get_block_size(best_block)>=size)
		{
			int diff=get_block_size(best_block)-size;
					if(diff>=16)// if(block size is bit larger)
					{
						//Create New Allocated Block
						struct BlockElement *AllocatedBlock;
						set_block_data(best_block,size,1);
						AllocatedBlock = best_block;
						//Create New Empty Block
						struct BlockElement *new_Empty_Block = (void *)AllocatedBlock + size;
						set_block_data(new_Empty_Block,diff,0);
						//Adjust Previous Pointer
						LIST_INSERT_BEFORE(&freeBlocksList,best_block,new_Empty_Block);
						LIST_REMOVE(&freeBlocksList,best_block);



						return AllocatedBlock;

					}
					else
					{
						LIST_REMOVE(&freeBlocksList,best_block);
						set_block_data(best_block,size + diff,1);
						return best_block;
					}
			}
			uint16 no_of_pages = ROUNDUP(size, PAGE_SIZE)/PAGE_SIZE;
			void *da_break = sbrk(no_of_pages);
			if((int)da_break ==-1) return NULL;
			if (LIST_SIZE(&freeBlocksList) != 0)
			{
				block = LIST_LAST(&freeBlocksList);
				void *ptr = (void *) block + get_block_size(block);
				if(ptr == da_break)
					set_block_data(block, get_block_size(block) + no_of_pages*PAGE_SIZE, 0);
				else
				{
					set_block_data(da_break,no_of_pages*PAGE_SIZE, 0);// the error in free
										struct BlockElement * new_block = da_break;
										LIST_INSERT_TAIL(&freeBlocksList, new_block);
				}
			}
			else
			{
				set_block_data(da_break, no_of_pages*PAGE_SIZE, 0);
				struct BlockElement * new_block = da_break;
				LIST_INSERT_TAIL(&freeBlocksList, new_block);
			}
			da_break+=no_of_pages*PAGE_SIZE;
			uint32 *end_block = da_break - 4;
			*end_block = 1;
			return alloc_block_BF(size - 8);
}
//===================================================
// [5] FREE BLOCK WITH COALESCING:
//===================================================
void free_block(void *va)
{
	//TODO: [PROJECT'24.MS1 - #07] [3] DYNAMIC ALLOCATOR - free_block
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("free_block is not implemented yet");
	//Your Code is Here...
	va=(struct BlockElement*)va;
	if ((uint32*)va == '\0' ) return;

	    uint32* prev_foot = (uint32 *)((uint8 *)va - 4);//we point on the header of current va because we will decrement pointer by 4 bytes in get size
		uint32* next_head = (uint32 *)((uint8 *)va + get_block_size(va));// it will be point on va of next block
		int8 isprev_free = is_free_block(prev_foot);// is prev block is free or not
		int8 isnext_free = is_free_block(next_head);// is next block is free or not
		//0: allocated 1: free
		if(isprev_free == 1 && isnext_free == 0)
		{
			//free block behind current address

			//get address of previous block
			uint32 *prevVA =(uint32 *)( (uint8 *)prev_foot - get_block_size(prev_foot) + 4);
			//set the size of previous block to be previous size + current block size
			// which after change the header size so the prevVA will point on the new block
			set_block_data(prevVA, get_block_size(va) + get_block_size(prevVA) ,0);

		}else if(isprev_free == 0 && isnext_free  == 1)
		{   //free block in front of current address


			set_block_data(va,get_block_size(va) + get_block_size(next_head),0);
			// we will add the new free block before the next block which  is free block so we merg an change the header of current va and add the new block in free list and remove next block from free list
			LIST_INSERT_BEFORE(&freeBlocksList,(struct BlockElement *)next_head,(struct BlockElement *)va);
			LIST_REMOVE(&freeBlocksList,(struct BlockElement *)next_head);

		}else if(isprev_free == 0 && isnext_free == 0)
		{//no merge the next and previous blocks is allocated

			// we will set block data with the size required and make it free and add it to list
			set_block_data(va,get_block_size(va),0);

// 			this is the code which get trap frame error
//			struct BlockElement *block;
//			struct BlockElement *foundBlock;
//			LIST_FOREACH(block, &freeBlocksList) {
//						if(block < (struct BlockElement *)va )
//						{
//							foundBlock = block;
//							cprintf("if 3.4 call\n");
//						}
//					}
//					if(foundBlock == NULL){
//						LIST_INSERT_HEAD(&freeBlocksList,((struct BlockElement *)va ) );
//						cprintf("if 3.41 call\n");
//					}
//					else {
//
//						LIST_INSERT_AFTER(&freeBlocksList,foundBlock,(struct BlockElement *)va);
//						cprintf("if 3.42 call\n");
//					}

			// it get trap frame error when we iterate on free block list to now where to put new block to keep it sorted
			//LIST_INSERT_TAIL(&freeBlocksList,(struct BlockElement *)va);
			struct BlockElement *ptr;
			LIST_FOREACH(ptr, &freeBlocksList)
				if ((int)va < (int)ptr)break;
			if (ptr != NULL)
				LIST_INSERT_BEFORE(&freeBlocksList, ptr, (struct BlockElement *) va);
			else
				LIST_INSERT_TAIL(&freeBlocksList,(struct BlockElement *)va);


		}else if(isprev_free == 1 && isnext_free == 1){
			//merge left and right

			// we will change the header of previous block so make it size = previous block.size  +the required free block + the next block
			//get address of previous block
			uint32 *prevVA =(uint32 *)( (uint8 *)va - get_block_size(prev_foot));

			//change the header of previous block to the total size and make it free
			// the prevVA is store in list and point on the hole new block
			set_block_data(prevVA,get_block_size(prev_foot)+get_block_size(va)+get_block_size(next_head),0);

			// will remove the next block which is free
			if (next_head=='\0' || next_head)
			LIST_REMOVE(&freeBlocksList,(struct BlockElement *)next_head);

		}
}
//=========================================
// [6] REALLOCATE BLOCK BY FIRST FIT:
//=========================================
void *realloc_block_FF(void* va, uint32 new_size)
{
	//edge cases
	if (new_size==0 && va !=NULL){
		free_block(va);
		return NULL;
	}
	if(new_size!=0 && va==NULL)
		return alloc_block_FF(new_size);

	if(new_size==0 && va ==NULL)
		return NULL;
	if(is_free_block(va)==1)
		return NULL;

	new_size+=8; //add header and footer to incoming size
	struct BlockElement *current_block=va;
	struct BlockElement * next_block = (void *) va + get_block_size(va);
	uint32 current_size=get_block_size(va);
	uint32 next_size=get_block_size(next_block);

	if(current_size==new_size)
		return current_block;

	//case if size block will be increasing
	if (current_size < new_size)
	{
		uint32 size_deficit=new_size-current_size;

		//case if next block is already free
		if(is_free_block(next_block))
		{
			//case if next size is bigger than the difference then we should reallocate in place and take
			//size from the block in front
			if(next_size>=size_deficit)
			{
				struct BlockElement * previous_free= LIST_PREV(next_block);
				LIST_REMOVE(&freeBlocksList,next_block);
				//case if we should split block (after taking from the block in front the size will be bigger than 16)
				//else it should have internal fragmentation and not create another free block
				if(next_size-size_deficit>=16)
				{
					set_block_data(va,new_size,1);
					next_block = (void *) va + get_block_size(va);
					set_block_data(next_block,next_size-size_deficit,0);
					if(previous_free==NULL)
						LIST_INSERT_HEAD(&freeBlocksList,next_block);
					else
						LIST_INSERT_AFTER(&freeBlocksList,previous_free,next_block);
				}
				else
					set_block_data(va,current_size+next_size,1);
				return va;
			}
			else //if there's not enough space to allocate from next free block then we should free our
				//space and alloc_ff the new size
			{
				void* new_block_VA=alloc_block_FF(new_size-8);
				if(new_block_VA==NULL)return va;
				memmove(new_block_VA,va,new_size-8);
				free_block(va);
				return new_block_VA;
			}
		}
		else// if the next block is not free then we should free block our current address then alloc_ff with new size
		{
			void* new_block_VA=alloc_block_FF(new_size-8);
			if(new_block_VA==NULL)return va;
			memmove(new_block_VA,va,new_size-8);
			free_block(va);
			return new_block_VA;
		}
	}
	//case if we're reallocating but decreasing size
	else
	{
		uint32 surplus_size=current_size-new_size;

		//edge case if we're trying to resize to something smaller than the minimum size
		if(new_size<16)
			return va;
		//case if the block in front of us is free then after down sizing our current block we need to merge
		//the discarded size with the block in front of us
		if(is_free_block(next_block))
		{
			set_block_data(va,new_size,1);
			struct BlockElement *previous_block= LIST_PREV(next_block);
			LIST_REMOVE(&freeBlocksList, next_block);
			next_block = (void *) next_block - surplus_size;
			set_block_data(next_block, next_size+surplus_size, 0);
			if(previous_block==NULL)
				{LIST_INSERT_HEAD(&freeBlocksList, next_block);
				}
			else
				{LIST_INSERT_AFTER(&freeBlocksList,previous_block,next_block);}
		}
		//case if the next block is not free then we split and create a new free block from discarded size
		else
		{
			if (surplus_size >= 16)
			{
				struct BlockElement * ptr;
				LIST_FOREACH(ptr,&freeBlocksList)
				{
					if((struct BlockElement *)va<ptr)
						break;
				}
				struct BlockElement *splitted_block= (void *) va + new_size;
				set_block_data(va,new_size,1);
				set_block_data(splitted_block,surplus_size,0);
				if(ptr==NULL)
					LIST_INSERT_TAIL(&freeBlocksList,splitted_block);
				else
					LIST_INSERT_BEFORE(&freeBlocksList,ptr,splitted_block);
				return va;
			}
		}
		return va;
	}
	return va;

}
/*********************************************************************************************/
/*********************************************************************************************/
/*********************************************************************************************/
//=========================================
// [7] ALLOCATE BLOCK BY WORST FIT:
//=========================================
void *alloc_block_WF(uint32 size)
{
	panic("alloc_block_WF is not implemented yet");
	return NULL;
}

//=========================================
// [8] ALLOCATE BLOCK BY NEXT FIT:
//=========================================
void *alloc_block_NF(uint32 size)
{
	panic("alloc_block_NF is not implemented yet");
	return NULL;
}
