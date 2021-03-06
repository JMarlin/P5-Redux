#ifndef MEMORY_H
#define MEMORY_H

#define MAX_KERNEL_HEAP 0x006FFFFF

typedef struct memblock {
    void* base;
    unsigned int size;
    struct memblock* next;
} memblock;

typedef struct memzone {
    unsigned long long int base;
    unsigned long long int length;
    unsigned int type;
} memzone;

void init_memory();

//Find the end memblk and update it to allocate size + sizeof(memblk)
//bytes of memory, returning the pointer to the memory at the base of
//the new region + sizeof(memblk).
void* kmalloc(unsigned int size);

//Look through the memblk chain until we find  a pointer - sizeof(memblk)
//memblk pointer. Then delete the entry from the chain and null the
//pointer
void* kfree(void* pointer);

#endif
