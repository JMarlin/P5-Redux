#ifndef MEMORY_H
#define MEMORY_H

typedef struct memblock {
    void* base;
    unsigned int size;
    struct memblock* next;
} memblock;

void* malloc(unsigned int byte_count);
void free(void* base_addr);
void* realloc(void* old_address, unsigned int byte_count);

#endif //MEMORY_H