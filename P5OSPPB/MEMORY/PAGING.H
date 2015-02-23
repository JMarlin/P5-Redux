#ifndef PAGING_H
#define PAGING_H

void init_mmu();
void* page_proc_map(unsigned int procBase, unsigned int pageCount, char protect);
void page_proc_free(void* procBase);
void* page_proc_last(unsigned int procBase);

#endif //PAGING_H
