#include <inc/memlayout.h>
#include "shared_memory_manager.h"

#include <inc/mmu.h>
#include <inc/error.h>
#include <inc/string.h>
#include <inc/assert.h>
#include <inc/queue.h>
#include <inc/environment_definitions.h>

#include <kern/proc/user_environment.h>
#include <kern/trap/syscall.h>
#include "kheap.h"
#include "memory_manager.h"

//==================================================================================//
//============================== GIVEN FUNCTIONS ===================================//
//==================================================================================//
struct Share* get_share(int32 ownerID, char* name);
struct spinlock create_lock;
//===========================
// [1] INITIALIZE SHARES:
//===========================
//Initialize the list and the corresponding lock

void sharing_init()
{
#if USE_KHEAP
	LIST_INIT(&AllShares.shares_list) ;
	init_spinlock(&AllShares.shareslock, "shares lock");
	init_spinlock(&create_lock ,"create lock");
#else
	panic("not handled when KERN HEAP is disabled");
#endif
}

//==============================
// [2] Get Size of Share Object:
//==============================
int getSizeOfSharedObject(int32 ownerID, char* shareName)
{
	//[PROJECT'24.MS2] DONE
	// This function should return the size of the given shared object
	// RETURN:
	//	a) If found, return size of shared object
	//	b) Else, return E_SHARED_MEM_NOT_EXISTS
	//
	struct Share* ptr_share = get_share(ownerID, shareName);
	if (ptr_share == NULL)
		return E_SHARED_MEM_NOT_EXISTS;
	else
		return ptr_share->size;

	return 0;
}

//===========================================================


//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//
//===========================
// [1] Create frames_storage:
//===========================
// Create the frames_storage and initialize it by 0
inline struct FrameInfo** create_frames_storage(int numOfFrames)
{
	//TODO: [PROJECT'24.MS2 - #16] [4] SHARED MEMORY - create_frames_storage()
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("create_frames_storage is not implemented yet");
	//Your Code is Here...

	struct FrameInfo** frames_storage = (struct FrameInfo**)kmalloc(numOfFrames * sizeof(struct FrameInfo*));

	if (frames_storage == NULL)
		return NULL;

	 memset(frames_storage, 0, numOfFrames * sizeof(struct FrameInfo*));

//	for(int i = 0; i < numOfFrames ; i++)
//	{
//		frames_storage[i] = 0;
//	}
	    return frames_storage;

}

//=====================================
// [2] Alloc & Initialize Share Object:
//=====================================
//Allocates a new shared object and initialize its member
//It dynamically creates the "framesStorage"
//Return: allocatedObject (pointer to struct Share) passed by reference
struct Share* create_share(int32 ownerID, char* shareName, uint32 size, uint8 isWritable)
{
	//TODO: [PROJECT'24.MS2 - #16] [4] SHARED MEMORY - create_share()
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("create_share is not implemented yet");
	//Your Code is Here...

	    struct Share* new_share = (struct Share*)kmalloc(sizeof(struct Share));

	    if (new_share == NULL)
	    {
	        return NULL;
	    }

	    new_share->ownerID = ownerID;
	    if(strlen(shareName) > 63)
	    {
	    	// undo
	    	kfree((void*)new_share);
	    	return NULL;
	    }

	    strcpy(new_share->name,shareName);
	    new_share->size = size;
	    new_share->isWritable = isWritable;
	    new_share->references = 1;
        // share_id after masking most significant bit
	    new_share->ID = (uint32)new_share & 0x7FFFFFFF;

        int numOfFrames =  ROUNDUP(size, PAGE_SIZE)/PAGE_SIZE;
	    new_share->framesStorage = create_frames_storage(numOfFrames);
	    if(new_share->framesStorage == NULL)
	    {
	    	// undo
	        kfree((void*)new_share);
	        return NULL;
	    }

	    init_spinlock(&new_share->FramesLock ,shareName); // initialize spin lock of own frame
	    return new_share;
}
//=============================
// [3] Search for Share Object:
//=============================
//Search for the given shared object in the "shares_list"
//Return:
//	a) if found: ptr to Share object
//	b) else: NULL
struct Share* get_share(int32 ownerID, char* name)
{
	//TODO: [PROJECT'24.MS2 - #17] [4] SHARED MEMORY - get_share()
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("get_share is not implemented yet");
	//Your Code is Here...

	acquire_spinlock(&AllShares.shareslock);
   if(LIST_SIZE(&AllShares.shares_list) != 0)
   {
	   struct Share *share;
	   LIST_FOREACH(share, &AllShares.shares_list)
	   	{
	   	    if (share->ownerID == ownerID && strcmp(share->name, name) == 0)
	   	    {
	   	    	release_spinlock(&AllShares.shareslock);
	   	        return share;
	   	    }
	   	}

   }

	release_spinlock(&AllShares.shareslock);
	    return NULL;
}


//=========================
// [4] Create Share Object:
//=========================
int createSharedObject(int32 ownerID, char* shareName, uint32 size, uint8 isWritable, void* virtual_address)
{
	//TODO: [PROJECT'24.MS2 - #19] [4] SHARED MEMORY [KERNEL SIDE] - createSharedObject()
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("createSharedObject is not implemented yet");
	//Your Code is Here...

	acquire_spinlock(&create_lock);

	struct Env* myenv = get_cpu_proc(); //The calling environment

	if(get_share(ownerID,shareName) != NULL) // if sahred object exist already return E_SHARED_MEM_EXISTS
	{
		release_spinlock(&create_lock);
		return E_SHARED_MEM_EXISTS ;
	}

	struct Share *new_shared_object = create_share(ownerID, shareName, size, isWritable);

	if(new_shared_object==NULL) // if the create share failed to create return E_NO_SHARE
	{
		release_spinlock(&create_lock);
		return E_NO_SHARE;
	}

	uint32 NoOfFrames = ROUNDUP(size, PAGE_SIZE)/PAGE_SIZE; // number of object's frame
	uint32* ptr_page_table;
	struct FrameInfo *frame;

	for (int i=0; i<NoOfFrames; i++)
	{
		int ret = get_page_table(myenv->env_page_directory,(uint32)virtual_address+i*PAGE_SIZE,&ptr_page_table);
		if(ret == TABLE_NOT_EXIST)
		{
			create_page_table(myenv->env_page_directory,(uint32)virtual_address+i*PAGE_SIZE);
		}
		allocate_frame(&frame);
		map_frame(myenv->env_page_directory , frame , (uint32)virtual_address + i*PAGE_SIZE ,PERM_USER|PERM_WRITEABLE);
		new_shared_object->framesStorage[i] = frame; // keep track of the physical frame
	}

    // insert the created object in the shares list
	acquire_spinlock(&AllShares.shareslock);
	LIST_INSERT_TAIL(&AllShares.shares_list, new_shared_object);
	release_spinlock(&AllShares.shareslock);

	release_spinlock(&create_lock);

	return new_shared_object->ID;
}


//======================
// [5] Get Share Object:
//======================
int getSharedObject(int32 ownerID, char* shareName, void* virtual_address)
{
	//TODO: [PROJECT'24.MS2 - #21] [4] SHARED MEMORY [KERNEL SIDE] - getSharedObject()
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("getSharedObject is not implemented yet");
	//Your Code is Here...


	struct Env* myenv = get_cpu_proc(); //The calling environment

	struct Share* shared_object = get_share(ownerID, shareName);
	// if shared not exist in shares list return E_SHARED_MEM_NOT_EXISTS
	if(shared_object == NULL)
	{
	    return E_SHARED_MEM_NOT_EXISTS;
	}
	else
	{
		acquire_spinlock(&shared_object->FramesLock); // acquire the lock of object's frame

		uint32 NoOfFrames = ROUNDUP(shared_object->size, PAGE_SIZE)/PAGE_SIZE; // number of frames of this object
        // if any virtual address not has page table create page table
		uint32* ptr_page_table;
		// if the object allow writable permission
		if(shared_object->isWritable == 1)
		{
			for(int i = 0; i < NoOfFrames ; i++)
			{
				int tableStatus = get_page_table(myenv->env_page_directory,(uint32)virtual_address+i*PAGE_SIZE,&ptr_page_table);
				if(tableStatus == TABLE_NOT_EXIST)
				{
					create_page_table(myenv->env_page_directory,(uint32)virtual_address+i*PAGE_SIZE);
				}
				// map on the frames of the owner object
				map_frame(myenv->env_page_directory , shared_object->framesStorage[i] , (uint32)virtual_address + i*PAGE_SIZE , PERM_USER|PERM_WRITEABLE);
			}
		}
		else
		{
			for(int i = 0; i < NoOfFrames ; i++)
			{
				int tableStatus = get_page_table(myenv->env_page_directory,(uint32)virtual_address+i*PAGE_SIZE,&ptr_page_table);
				if(tableStatus == TABLE_NOT_EXIST)
				{
					create_page_table(myenv->env_page_directory,(uint32)virtual_address+i*PAGE_SIZE);
				}
				map_frame(myenv->env_page_directory , shared_object->framesStorage[i] , (uint32)virtual_address+ i*PAGE_SIZE , PERM_USER);
			}
		}

        // increment references of the shared object
	    shared_object->references++;
	    release_spinlock(&shared_object->FramesLock);// release the lock of object's frame

	}

return shared_object->ID;
}
//==================================================================================//
//============================== BONUS FUNCTIONS ===================================//
//==================================================================================//

//==========================
// [B1] Delete Share Object:
//==========================
//delete the given shared object from the "shares_list"
//it should free its framesStorage and the share object itself
void free_share(struct Share* ptrShare)
{
	//TODO: [PROJECT'24.MS2 - BONUS#4] [4] SHARED MEMORY [KERNEL SIDE] - free_share()
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("free_share is not implemented yet");
	//Your Code is Here...
    acquire_spinlock(&AllShares.shareslock);
	LIST_REMOVE(&AllShares.shares_list,ptrShare);
	release_spinlock(&AllShares.shareslock);
	kfree((void*)ptrShare->framesStorage); // free frames stroage
	kfree((void*)ptrShare); // free share itself
}
//========================
// [B2] Free Share Object:
//========================
int freeSharedObject(int32 sharedObjectID, void *startVA)
{
	//TODO: [PROJECT'24.MS2 - BONUS#4] [4] SHARED MEMORY [KERNEL SIDE] - freeSharedObject()
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("freeSharedObject is not implemented yet");
	//Your Code is Here...


	struct Env *myenv = get_cpu_proc(); //The calling environment
    assert(myenv != NULL);

    // get the object from shares list with object id
    acquire_spinlock(&AllShares.shareslock);
	struct Share *share;
	struct Share *share_itr; // iterator for searching in list with ID

	bool found = 0;
	if(LIST_SIZE(&AllShares.shares_list) != 0)
	{
		LIST_FOREACH(share_itr, &AllShares.shares_list)
		{
			if (share_itr->ID == sharedObjectID)
	        {
				share = share_itr;
				found = 1;
                break;
			}
	    }
	}
    release_spinlock(&AllShares.shareslock);
    // if the object not found return E_SHARED_MEM_NOT_EXISTS
    if(!found)
    {
    	return E_SHARED_MEM_NOT_EXISTS;
    }
    acquire_spinlock(&share->FramesLock);// acquire the lock of object's frame

    struct FrameInfo * frame;

    int NoOfFrames = ROUNDUP(share->size,PAGE_SIZE)/PAGE_SIZE; // numebr of frame of the owner object
    uint32 va = (uint32)startVA;

    for(int i = 0 ; i <NoOfFrames ; i++)
    {

     uint32 *ptr_page_table;
     uint32 tableStatus = get_page_table(myenv->env_page_directory,va,&ptr_page_table);
     // if this virtual address not has page table return 0
     if(tableStatus == TABLE_NOT_EXIST)
     {
    	 release_spinlock(&share->FramesLock);
    	 return 0;
     }
     // if this the last object (owner) set his frames storage = 0 (null) and free this frame
     if(share->references == 1)
	 {
    	 struct FrameInfo* frame = share->framesStorage[i];
    	 free_frame(frame);
    	 share->framesStorage[i] = 0;
	 }
     // unmap this vitual address from the object's frames
     unmap_frame(myenv->env_page_directory,va);
       // check if the page table become empty
       // then remove it and set the his entry in page directory null entry
    	bool empty = 1;
    	for(int i = 0 ; i < 1024 ; i++)
    	{
    		if(ptr_page_table[i] != 0)
    		{
    			empty = 0;
    			break;
    		}
    	}

    	if(empty)
    	{
    		kfree((void*)ptr_page_table);
    		myenv->env_page_directory[PDX(va)] = 0;
    	}

    	va += PAGE_SIZE;

    }

    share->references--; // decrement refernces of this shared object

   // if this the last object free the share & free his frames storage & remove it from shares list
    if(share->references == 0)
    {
       free_share(share);
    }

    release_spinlock(&share->FramesLock); // release the lock of object's frame

    tlbflush(); // finally flush the cache

    return 1; // retrun 1 if success

}
