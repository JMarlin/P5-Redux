#ifndef MEMORY_H
#define MEMORY_H

void init_memory();

//Find the end memblk and update it to allocate size + sizeof(memblk) 
//bytes of memory, returning the pointer to the memory at the base of
//the new region + sizeof(memblk).
void* kmalloc(unsigned long size);

//Look through the memblk chain until we find  a pointer - sizeof(memblk)
//memblk pointer. Then delete the entry from the chain and null the 
//pointer
void* kfree(void* pointer);

#endif
