#include "../include/p5.h"
#include "../include/memory.h"

typedef struct memblock {
    void* base;
    unsigned int size;
    struct memblock* next;
	struct memblock* prev;
} memblock;

memblock* root_block = (memblock*)0;

void (*log_start)(void) = 0;
void (*log_end)(void) = 0;

void enable_debug(void (*cb_a)(void), void (*cb_b)(void)) {

    log_start = cb_a;
    log_end = cb_b;
}

//This is currently very naive implementation which does not allow for
//free()ing and allocates new chunks of memory in a completely linear 
//fashion

//We store memblocks linked in order of ascending address, 
//So this just looks through the list and finds the first gap
//between two linked blocks
//Oops. I guess this is just going to replace the old malloc code
void* malloc(unsigned int requested_size) {
	
	memblock* current_block;
	memblock* new_block;
	memblock* last_block = (memblock*)0;
	void* free_base;
	unsigned int available_space = 0;
	static unsigned int allocated_pages = 0;

        if(log_start) log_start();
	
	requested_size += sizeof(memblock);
	current_block = root_block;
	
	while(current_block) {
		
		if(current_block->prev) {
			
			available_space = (unsigned int)current_block->base - ((unsigned int)current_block->prev->base + current_block->prev->size);
			
			if(available_space >= requested_size) {
			    
				//Set up the new block 
				new_block = (memblock*)((unsigned int)current_block->prev->base + current_block->prev->size);
				new_block->base = (void*)new_block;
				new_block->size = requested_size;
				
				//Insert it into the chain
				new_block->prev = current_block->prev;
				new_block->next = current_block;
				current_block->prev->next = new_block;
                current_block->prev = new_block;
				
				//And return its usable base 
                                if(log_end) log_end();
				return (void*)((unsigned int)new_block->base + sizeof(memblock));
			}
		}
		
		last_block = current_block;
		current_block = current_block->next;
	}
	
	//No interleaving space found,
	//Check for space at the end of the memory area
	if(last_block) {
		
		//If we have an end block, we can get the first
		//memory location after it
		free_base = (void*)((unsigned int)last_block->base + last_block->size);
	} else {
	
		//Otherwise, we need to figure out what the start of usable space is
		free_base = (void*)(getImageSize(getCurrentPid()) + 0xB00000);
	} 
	
	//Now that we have start of memory, we can check to see if we need
	//additional space allocated by the OS or not to contain the new data
	available_space = ((((unsigned int)free_base) / 0x1000) * 0x1000) + ((((unsigned int)free_base) % 0x1000 > 0) ? 0x1000 : 0 ) - ((unsigned int)free_base); 
	
	//If we don't have enough space, pop a new page on 
	while(available_space < requested_size) {
				
		if(!appendPage()) {

                        if(log_end) log_end();
			return (void*)0;
                }
		
		allocated_pages++;
		available_space += 0x1000;
	}
	
	//and then create the new memory block, assigning it to the preceeding block 
	//next pointer or to the root pointer if there were no preceeding blocks
	new_block = (memblock*)((unsigned int)free_base);
	new_block->base = (void*)new_block;
	new_block->size = requested_size;
	new_block->next = (memblock*)0;
	
	if(last_block) {

		new_block->prev = last_block;
		last_block->next = new_block;
	} else {
	
		new_block->prev = (memblock*)0;
		root_block = new_block;
	} 
	
        if(log_end) log_end();
	return (void*)((unsigned int)new_block->base + sizeof(memblock));
}

memblock* find_memblock(void* address) {
	
	memblock* cur_block = root_block;
	
	while(cur_block) {
		
		if(cur_block->base == (address - sizeof(memblock)))
			break;
			
		cur_block = cur_block->next;
	}
	
	return cur_block;
}

void free(void* address) {
	
	memblock* to_delete = find_memblock(address);
	
	if(!to_delete)
	    return;
    
	//To delete a memblock, just remove it from the chain 
	to_delete->prev->next = to_delete->next;
	to_delete->next->prev = to_delete->prev;
}

void* memcpy(void* old_address, void* new_address, int count) {

    //NOTE: Current form assumes a transfer of longs on long 
	//boundaries, needs to be updated to conform to start and
	//count

	__asm__ __volatile__ ( 
		"movl %0, %%ecx \n\t"
		"movl %2, %%esi \n\t"
		"movl %%esi, %%edi \n\t"
		"movl %1, %%esi \n\t"
		"movl %%ds, %%es \n\t"
		"shrl $2, %%ecx \n\t"
		"andl $0xFFFFFFFC, %%esi \n\t"
		"andl $0xFFFFFFFC, %%edi \n\t"
		"rep movsl   \n\t" : : "r"(count), "r"(old_address), "r"(new_address) : "%esi", "%edi", "%ecx");
}

void* realloc(void* old_address, unsigned int byte_count) {
	
	int i;
	unsigned int copy_size;
	unsigned char *oldbuf, *newbuf;
	
	//Get the original memblock
	memblock* old_block = find_memblock(old_address);
	
	if(!old_block)
		return (void*)0;
	
	//Malloc the new space
	void* new_address = malloc(byte_count);
	
	if(!new_address)
		return (void*)0;
	
	//Set up the transfer buffers
	newbuf = (unsigned char*)new_address;
	oldbuf = (unsigned char*)old_address;
	
	//Figure out how many bytes we're copying
	if(byte_count < old_block->size)
		copy_size = byte_count;
	else
		copy_size = old_block->size;
	
	//Copy the old content to the new location (again, could probably be much improved by a hardware-aware memcpy routine)
	for(i = 0; i < copy_size; i++)
		newbuf[i] = oldbuf[i];
		
	//Delete the old allocation
	free(old_address);
	
	return new_address;
}
