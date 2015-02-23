#ifndef PAGING_H
#define PAGING_H

typedef struct pageRange {
    int base_page;
    int count;
    struct pageRange* next;
} pageRange;

void init_mmu();
void free_pages(unsigned int physBase, unsigned int size);
void map_pages(unsigned int physBase, unsigned int virtBase, unsigned int size, unsigned short flags);
pageRange* new_page_tree(unsigned int pageCount);
void disable_page_range(pageRange* pr_base);
void apply_page_range(unsigned int vBase, pageRange* pr_base);
int append_page(pageRange* pr_base);

#endif //PAGING_H
