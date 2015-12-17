#include "../include/p5.h"
#include "../include/memory.h"

typedef struct memblock {
    void* base;
    unsigned int size;
    struct memblock* next;
	struct memblock* prev;
} memblock;

memblock* root_block = (memblock*)0;

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
		
		if(!appendPage())
			return (void*)0;
		
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
	
	return (void*)((unsigned int)new_block->base + sizeof(memblock));
}

/*
OLD CODE, so's we can figure out what we screwed up if anything with the new version
void* malloc(unsigned int byte_count) {
	
	//Make sure we set up the environment the first time malloc is called
	static unsigned char initialized = 0;
	static void* free_base = (void*)0;
	static unsigned int allocated_pages = 0;
	unsigned int trailing_space;
	memblock* current_block;
	memblock* new_block;
	
	if(!initialized) {
		
		//Get the address of the first byte past the executable image
		free_base = (void*)(getImageSize(getCurrentPid()) + 0xB00000);
		
		//Check to make sure we have enough space to allocate the memory
		trailing_space = ((((unsigned int)free_base) / 0x1000) * 0x1000) + ((((unsigned int)free_base) % 0x1000 > 0) ? 0x1000 : 0 ) - ((unsigned int)free_base); 
		
		//If we don't have enough space, pop a new page on 
		if(trailing_space < (byte_count + sizeof(memblock))) {
			
			if(!appendPage())
				return (void*)0;
			
			allocated_pages++;
		}
		
		//Instead of appending a new block to the end of the current chain, we need to 
		root_block = (memblock*)free_base;
		//make a new root block
		root_block->base = free_base;
		root_block->size = byte_count + sizeof(memblock);
		root_block->next = (memblock*)0;
		initialized = 1;
	
		return (void*)((unsigned int)root_block->base + sizeof(memblock));
	} else {
		
		current_block = root_block;
		
		while(current_block->next)
			current_block = current_block->next;
			
		trailing_space = (unsigned int)current_block->base + current_block->size;
		trailing_space = ((trailing_space / 0x1000) * 0x1000) + ((trailing_space % 0x1000 > 0) ? 0x1000 : 0 ) - trailing_space; 
		
		//If we don't have enough space, pop a new page on 
		if(trailing_space < (byte_count + sizeof(memblock))) {
			
			prints("[malloc] appending page\n");
			
			if(!appendPage()) {
				
				prints("[malloc] page request failed\n");
				return (void*)0;
			}
			
			prints("[malloc] page appended\n");
			
			allocated_pages++;
		}
		
		new_block = (memblock*)((unsigned int)current_block->base + current_block->size);
		current_block->next = new_block;
		new_block->base = (void*)((unsigned int)current_block->base + current_block->size);
		new_block->size = byte_count + sizeof(memblock);
		new_block->next = (memblock*)0;
		
		return (void*)((unsigned int)new_block->base + sizeof(memblock));
	}
}
*/

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