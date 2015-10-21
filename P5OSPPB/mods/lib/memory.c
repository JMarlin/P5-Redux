#include "../include/p5.h"
#include "../include/memory.h"

//This is currently very naive implementation which does not allow for
//free()ing and allocates new chunks of memory in a completely linear 
//fashion
void* malloc(unsigned int byte_count) {
	
	//Make sure we set up the environment the first time malloc is called
	static unsigned char initialized = 0;
	static void* free_base = (void*)0;
	static unsigned int allocated_pages = 0;
	static memblock* root_block = (memblock*)0;
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
		//make a new root block
		root_block = (memblock*)free_base;
		root_block->base = free_base;
		root_block->size = byte_count + sizeof(memblock);
		root_block->next = (memblock*)0;
		initialized = 1;
	
		return (void*)((unsigned int)root_block->base + sizeof(memblock));
	} else {
		
		current_block = root_block;
		
		while(current_block->next)
			current_block = current_block->next;
			
		trailing_space = (unsigned int)curent_block->base + current_block->size;
		trailing_space = ((trailing_space / 0x1000) * 0x1000) + ((trailing_space % 0x1000 > 0) ? 0x1000 : 0 ) - trailing_space; 
		
		//If we don't have enough space, pop a new page on 
		if(trailing_space < (byte_count + sizeof(memblock))) {
			
			if(!appendPage())
				return (void*)0;
			
			allocated_pages++;
		}
		
		new_block = (memblock*)(curent_block->base + (void*)current_block->size);
		current_block->next = new_block;
		new_block->base = curent_block->base + (void*)current_block->size;
		new_block->size = byte_count + sizeof(memblock);
		new_block->next = (memblock*)0;
		
		return (void*)((unsigned int)new_block->base + sizeof(memblock));
	}
}

void free(void* address) {
	
}