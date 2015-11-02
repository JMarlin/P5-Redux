#include "../include/p5.h"
#include "../include/memory.h"

memblock* root_block = (memblock*)0;

//This is currently very naive implementation which does not allow for
//free()ing and allocates new chunks of memory in a completely linear 
//fashion
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
			
			if(!appendPage())
				return (void*)0;
			
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
	
}

void* realloc(void* old_address, unsigned int byte_count) {
	
	int i;
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
	
	//Copy the old content to the new location (again, could probably be much improved by a hardware-aware memcpy routine)
	for(i = 0; i < old_block->size; i++)
		newbuf[i] = oldbuf[i];
		
	//Delete the old allocation
	free(old_address);
	
	return new_address;
}