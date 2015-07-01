#include "../ascii_io/ascii_o.h"
#include "../core/global.h"
#include "../memory/memory.h"
#include "paging.h"


extern void _loadPageDirectory(unsigned int*);
extern void _enablePaging();
extern unsigned long maxRAM; //Defined in memory.c
unsigned int *pageDirectory = (unsigned int*)0x200000;
unsigned int *pageTable = (unsigned int*)PAGE_TABLE_ADDRESS;

/*
    Bits 9-11 of each pageTable entry are used by the OS to keep track of
    certain memory details. Bit 11 (0x800) is used to indicate that the memory
    has been claimed for use by the OS. Bit 10 (0x400) is used to indicate that
    the memory has been marked 'special' by the OS, meaning that it is a special
    memory area or perhaps a zone for memory mapped IO and should NOT be used
    for general memory allocation
*/

void init_page_system() {

    int i, j;

    //Clear the 1,024 page directory entries
    for(i = 0; i < 1024; i++) {

        //Set to global access, write-enabled,
        //page present
        pageDirectory[i] = 0x00700007 + (i * 0x1000);

        for(j = 0; j < 1024; j++) {

            //Supervisor access, not present
            pageTable[(i * 1024) + j] = 0x00000002;
        }
    }
}


void free_pages(unsigned int physBase, unsigned int size) {

    int i = physBase >> 12;
    int max = i + (size >> 12);

    for( ; i < max; i++) {

        //Make sure to preserve the OS-status bits
        pageTable[i] &= 0x00000E00;
        pageTable[i] |= 0x00000002;
    }
}


void map_pages(unsigned int physBase, unsigned int virtBase, unsigned int size, unsigned short flags) {

    int i = virtBase >> 12;
    int max = i + (size >> 12);

    for( ; i < max; i++, physBase += 0x1000) {

        DEBUG("Mapping virt 0x");
        DEBUG_HD(i << 12);
        DEBUG(" -> phys 0x");
        DEBUG_HD(physBase);
        DEBUG("\n");

        //Some day, we'll figure this out. Some day.
        prints("\0");

        //Make sure to preserve the OS-status bits
        pageTable[i] &= 0x00000E00;
        pageTable[i] |= (physBase & 0xFFFFF000) | (flags & 0x1FF);
    }
}


pageRange* new_page_tree(unsigned int pageCount) {

    int i;
    pageRange* new_pr;

    if(!(new_pr = (pageRange*)kmalloc(sizeof(pageRange))))
        return (pageRange*)0x0;

    new_pr->base_page = 0;
    new_pr->count = 0;
    new_pr->next = (pageRange*)0x0;

    for(i = 0; i < pageCount; i++) {

        if(!append_page(new_pr)) {

            kfree((void*)new_pr);
            return (pageRange*)0x0;
        }
    }

    return new_pr;
}


void del_page_tree(pageRange* root_page) {

    int i = 0;
    int j, count;
    pageRange* current_pr = root_page;
    pageRange* last_pr;

    while(current_pr) {

        current_pr = current_pr->next;
        i++;
    }

    count = i;

    for(i = 0; i < count; i++) {
        current_pr = root_page;
        last_pr = (pageRange*)0;

        while(current_pr->next) {
            last_pr = current_pr;
            current_pr = current_pr->next;
        }

        //Unmark the page-in-use bit for each claimed page
        for(j = current_pr->base_page; j < current_pr->base_page + current_pr->count; j++)
            pageTable[j] &= 0xFFFFF7FF;

        if(last_pr) last_pr->next = (pageRange*)0;
        kfree((void*)current_pr);
    }
}


void disable_page_range(unsigned int vBase, pageRange* pr_base) {

    pageRange* pr_current = pr_base;
    unsigned int vPage = vBase;

    //Iterate through
    while(pr_current) {

        //free_pages(pr_current->base_page << 12, pr_current->count << 12);
        free_pages(vPage, pr_current->count << 12);
        vPage += pr_current->count << 12;
        pr_current = pr_current->next;
    }

    _loadPageDirectory(pageDirectory);
}


void apply_page_range(unsigned int vBase, pageRange* pr_base, char super) {

    pageRange* pr_current = pr_base;
    unsigned int v_addr;

    v_addr = vBase;

    if(!pr_current)
        prints("    PAGE TREE IS EMPTY!\n");

    //Iterate through
    while(pr_current) {

        //Map the page with kernel privilege if super is set, user if not
        map_pages(pr_current->base_page << 12, v_addr, pr_current->count << 12, super ? 3 : 7);
        v_addr += (pr_current->count << 12);
        pr_current = pr_current->next;
    }

    _loadPageDirectory(pageDirectory);
}


//This ends the search of the page table at 0x2000
//as opposed to 0x100000 because we're just assuming our
//system has 32 megs of memory for now
//Note: This will be updated to reflect real memory
//availibility via #37
int append_page(pageRange* pr_base) {

    unsigned total_count = 0;
    pageRange* pr_current = pr_base;
    unsigned int offset;
    unsigned long maxPages = maxRAM >> 12;
    int i;

    //Get to the end of the list
    while(pr_current->next) {
        total_count += pr_current->count;
        pr_current = pr_current->next;
    }

    //There is nothing allocated here yet, so we'll go
    //ahead and find the first free phys page
    if(!pr_current->count) {

        for(i = 0xB00; i < maxPages; i++) {

            //Check to make sure the page is not already alocated and/or special
            if(!(pageTable[i] & 0xC00))
                break;
        }

        if(i == maxPages)
            return 0;

        pr_current->count++;
        pr_current->base_page = i;
        pageTable[i] |= 0x00000800;
        return (total_count + pr_current->count) << 12;
    }

    offset =  pr_base->base_page + pr_base->count;

    //The physical page immediately following the last
    //allocated page is free, so we can go ahead and
    //just mark the memory as used and ratchet up the
    //count of this contiguous block
    //Check to make sure the page is not already alocated and/or special
    if(!(pageTable[offset] & 0xC00)) {

        //Bit 0x800, which is available to the OS,
        //indicates physical page availability.
        //We mark it to 1 to indicate that we're
        //claiming this page
        pr_current->count++;
        pageTable[offset] |= 0x00000800;
        return (total_count + pr_current->count) << 12;
    }

    //We can't allocate contiguous memory, so we have
    //to allocate a new pageRange
    if(!(pr_current->next = (pageRange*)kmalloc(sizeof(pageRange)))) {

        pr_current->next = (pageRange*)0x0;
        return 0;
    }

    //And then search for the next availible page to assign it to
    for(i = 0xB00; i < maxPages; i++) {

        //Check to make sure the page is not already alocated and/or special
        if(!(pageTable[i] & 0xC00))
            break;
    }

    //If we hit the top of memory, we append a null page range node and
    //return failure to the caller
    if(i == maxPages) {

        pr_current->next = (pageRange*)0x0;
        return 0;
    }

    //Otherwise, we were able to successfully find a free page, so we
    //store its page number and mark it allocated
    pr_current->count++;
    pr_current->base_page = i;
    pageTable[i] |= 0x00000800;
    return (total_count + pr_current->count) << 12;
}


void init_mmu() {

    //prints("\nInit page directory...");
    init_page_system();

    //map one 1:1 8mb region at the start of memory for reserved kernel
    //space
    DEBUG("Done\nMap first 8Mb...");

    //Map the first 1MB of pages as usr for v86 code
    map_pages(0x00000000, 0x00000000, 0x00100000, 7);

    //Map the pages for the kernel space
    map_pages(0x00100000, 0x00100000, 0x00A00000, 3);

    //DEBUG("Done\nMap user's 4Mb...");
    //Flags: user ram, R/W enable, page present
    //mapRegion(0x00800000, 0x00800000, 1, 7);
    DEBUG("Done\nLoading the page directory...");
    _loadPageDirectory(pageDirectory);
    DEBUG("Done\nEnabling paging...");
    _enablePaging();
    DEBUG("Done\n");
}
