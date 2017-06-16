#ifndef MEMORY_H
#define MEMORY_H

void enable_debug(void (*cb_a)(void), void (*cb_b)(void));
void* malloc(unsigned int byte_count);
void free(void* base_addr);
void* realloc(void* old_address, unsigned int byte_count);
int memcpy(void* old_address, void* new_address, unsigned long count);
void print_malloc_list();

#endif //MEMORY_H
