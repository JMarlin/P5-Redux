#ifndef MEMORY_H
#define MEMORY_H

void* malloc(unsigned int byte_count);
void free(void* base_addr);
void* realloc(void* old_address, unsigned int byte_count);

#endif //MEMORY_H