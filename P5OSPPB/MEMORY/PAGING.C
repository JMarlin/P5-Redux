#include "../ascii_io/ascii_o.h"
#include "../core/global.h"
#include "../memory/memory.h"
#include "paging.h"


extern void loadPageDirectory(unsigned int*);
extern void enablePaging();
unsigned int *pageDirectory = (unsigned int*)0x200000;
unsigned int *pageTable = (unsigned int*)0x700000;
unsigned int *pageMap = (unsigned int*)0x300000;


void init_page_system() {
    
    int i, j;        
    
    //Clear the 1,024 page directory entries
    for(i = 0; i < 1024; i++) {
    
        //Set to global access, write-enabled,
        //page present
        pageDirectory[i] = 0x00000007 + (i * 0x1000);
        
        for(j = 0; j < 1024; j++) {
            
            //Supervisor access, not present
            pageTable[(i * 1024) + j] = 0x00000002;
        }
    }
}


void free_pages(unsigned int physBase, unsigned int size) {

    int i = physBase >> 12;
    int max = i + (size >> 12);
        
    //Mark these pages as not present and supervisor
    //the supervisor should always have its pages in
    //memory, so if both of these are set then
    //we know that this page is a deactivated user page
    for( ; i < max; i++) {    
        pageTable[i] &= 0xFFFFF7FA;
    }
}


void map_pages(unsigned int physBase, unsigned int virtBase, unsigned int size, unsigned short flags) {

    int i = physBase >> 12;
    int max = i + (size >> 12);
        
    for( ; i < max; i++, virtBase += 0x1000) {   
        
        //Make sure to preserve the page-reserved bit
        pageTable[i] &= 0x00000E00;
        pageTable[i] |= (virtBase & 0xFFFFF000) | (flags & 0x1FF);
    }
}


pageRange* new_page_tree(unsigned int pageCount) {

    int i;
    pageRange* new_pr;
    
    if(!(new_pr = (pageRange*)kmalloc(sizeof(pageRange))));
        return (pageRange*)0x0;
    
    new_pr->base_page = pageBase;
    new_pr->count = 0; 
    new_pr->next = (pageRange*)0x0;
    
    for(i = 0; i < pageCount; i++) {
        
        if(!append_page(new_pr)) {
            
            kfree((void*)new_pr);
            return (pageRange*)0x0;
        }
    }
}


void disable_page_range(pageRange* pr_base) {

    pageRange* pr_current = pr_base;    
    
    //Iterate through 
    while(pr_current) {
    
        free_pages(pr_current->base_page << 12, pr_current->count << 12);  
        pr_current = pr_current->next;
    }
    
    loadPageDirectory(pageDirectory);
}


void apply_page_range(unsigned int vBase, pageRange* pr_base) {

    pageRange* pr_current = pr_base;
    unsigned int v_addr;
    
    v_addr = vBase;
    
    //Iterate through 
    while(pr_current) {
    
        map_pages(pr_current->base_page << 12, v_addr, pr_current->count << 12, 7);   
        v_addr += (pr_current->count << 12);
        pr_current = pr_current->next;
    }
    
    loadPageDirectory(pageDirectory);
}


//This ends the search of the page table at 0x2000
//as opposed to 0x100000 because we're just assuming our
//system has 32 megs of memory for now
int append_page(pageRange* pr_base) {

    unsigned total_count = 0;
    pageRange* pr_current = pr_base;
    unsigned int offset;
    int i;
    
    //Get to the end of the list
    while(pr_current->next) {
        total_count += pr_current->count;
        pr_current = pr_current->next;
    }
    
    //There is nothing allocated here yet, so we'll go
    //ahead and find the first free phys page
    if(!pr_current->count) {
    
        for(i = 0xB00; i < 0x2000; i++) {
        
            if(!(pageTable[i] & 0x800))
                break;
        }
    
        if(i == 0x2000)
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
    if(!(pageTable[offset] & 0x800)) {
        
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
    
    for(i = 0xB00; i < 0x2000; i++) {
        
        if(!(pageTable[i] & 0x800))
            break;
    }
    
    if(i == 0x2000) {
        
        pr_current->next = (pageRange*)0x0;
        return 0;
    }
            
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
    loadPageDirectory(pageDirectory);
    DEBUG("Done\nEnabling paging...");
    enablePaging();        
    DEBUG("Done\n");
}
