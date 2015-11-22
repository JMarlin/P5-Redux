#include "../ascii_io/ascii_o.h"
#include "../core/global.h"
#include "../memory/memory.h"
#include "../process/process.h"
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

static inline void invlpg(void* address)
{

    asm volatile ( "invlpg (%0)" : : "b"(address) : "memory" );
}

void free_pages(unsigned int physBase, unsigned int size) {

    int i = physBase >> 12;
    int max = i + (size >> 12);

    for( ; i < max; i++) {

        //Make sure to preserve the OS-status bits
        pageTable[i] &= 0x00000E00;
        pageTable[i] |= 0x00000002;

        //And invalidate the page
        invlpg((void*)(i << 12));
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

        //And invalidate the page
        invlpg((void*)(i << 12));
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

    //_loadPageDirectory(pageDirectory);
}

//Find the next availible free physical page (very inefficient)
unsigned int find_free_page() {

    unsigned long maxPages = maxRAM >> 12;
    unsigned int i;

    for(i = 0xB00; i < maxPages; i++) {

        //Check to make sure the page is not already alocated and/or special
        if(!(pageTable[i] & 0xC00))
            return i;
    }

    if(i == maxPages)
        return 0;
}

//Find the next availible set of contiguous pages of the required count
unsigned int find_free_pages(unsigned int count) {

    unsigned long maxPages = maxRAM >> 12;
    unsigned int i, j;
    unsigned int base_page = 0;

    for(i = 0xB00; i < maxPages; i++) {

        //Check to make sure the page is not already alocated and/or special
        if(!(pageTable[i] & 0xC00)) {
            
            base_page = i;
               
            for(j = i; j < i + count; j++) {
                
                if((pageTable[j] & 0xC00) || j == maxPages) 
                    break;
            }
            
            if(j == i + count)
                return base_page;   
        }
    }

    return 0;
}

//Find a free page, mark it in use, identity map it, and return its address
//This should be improved in the future, ideally by not identity mapping this
//page but instead taking two PIDs and appending the same page to both of
//their memory spaces/allocation trees
//This also needs a free, and we need to figure out a mechanism for that as
//we don't want one process unmapping the memory before the other is done with it
//A probable solution would be to reference count the memory. EG: Count a counter
//up by one for every PID which attaches to the shared page and, when freeing,
//decrease the counter and then unmap and mark the memory availible only if the
//counter has returned to zero
void* allocate_shared_pages(unsigned int count) {

    int i;
    unsigned int temp_pages = find_free_pages(count);

    if(!temp_pages)
        return (void*)temp_pages;

    //Identity map the pages, global use, and mark them in use
    for(i = temp_pages; i < temp_pages + count; i++) 
        pageTable[i] |= 0x800;
        
    map_pages(temp_pages << 12, temp_pages << 12, 0x1000 * count, 7);
    
    //Return the base address
    return (void*)(temp_pages << 12);
}

void* allocate_shared_page(void) {

    unsigned int temp_page = find_free_page();

    if(!temp_page)
        return (void*)temp_page;

    //Identity map the pages, global use, and mark them in use
    pageTable[temp_page] |= 0x800;
        
    map_pages(temp_page << 12, temp_page << 12, 0x1000, 7);
    
    //Return the base address
    return (void*)(temp_page << 12);
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
    unsigned int temp_page;

    //Get to the end of the list
    prints("[kernel] Getting end of page list\n");
    while(pr_current->next) {
        total_count += pr_current->count;
        pr_current = pr_current->next;
    }

    //There is nothing allocated here yet, so we'll go
    //ahead and find the first free phys page
    if(!pr_current->count) {

        prints("[kernel] Finding a free page for the new page range\n");
        if((temp_page = find_free_page()) < 1)
            return 0;

        pr_current->count++;
        pr_current->base_page = temp_page;
        pageTable[temp_page] |= 0x00000800;
        return (total_count + pr_current->count) << 12;
    }

    offset =  pr_base->base_page + pr_base->count;

    //The physical page immediately following the last
    //allocated page is free, so we can go ahead and
    //just mark the memory as used and ratchet up the
    //count of this contiguous block
    //Check to make sure the page is not already alocated and/or special
    if(!(pageTable[offset] & 0xC00)) {

        prints("[kernel] Using next free page\n");

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
    prints("[kernel] Allocating a new page range\n");
    if(!(pr_current->next = (pageRange*)kmalloc(sizeof(pageRange)))) {
        
        prints("[kernel] Couldn't allocate a new page range\n");
        pr_current->next = (pageRange*)0x0;
        return 0;
    }

    //And then search for the next availible page to assign it to
    prints("[kernel] Looking for the next availible page\n");
    if((temp_page = find_free_page()) < 1) {

        prints("[kernel] No more pages availible\n");
        //If we hit the top of memory, we append a null page range node and
        //return failure to the caller
        pr_current->next = (pageRange*)0x0;
        return 0;
    }

    prints("[kernel] Marking new page allocated\n");
    //Otherwise, we were able to successfully find a free page, so we
    //store its page number and mark it allocated
    pr_current->count++;
    pr_current->base_page = temp_page;
    pageTable[temp_page] |= 0x00000800;
    prints("[kernel] Returning appended page\n");
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

//We're not implementing this yet, and here's why:
void free_physical(unsigned int physBase, unsigned int size) {

    //reserve_physical may reserve an area marked with 0x400, or only the os-special
    //flag, which means that that area of memory was detected at boot-time to
    //not be safe for normal memory usage. This is fine, because reserve_physical
    //should be used to map the areas that a device has told us to use and therefore
    //can be trusted.
    //The PROBLEM, however, is that the previous state of this flag is not saved
    //and therefore if we were to naively reset the flags to 0x000 when clearing
    //we may lose the indication that that memory should not be allocated to
    //processes. So for now, we're just never going to allow this memory to
    //be reclaimed until I can think of a good way to memo which pages were
    //originally os-special
}

//Identity map the specified region of physical memory and mark it reserved
//As well, this ensures that the physical memory is not currently occupied
//or reserved. If occupied, the memory content is moved to a free physical range
//and the page tree which maps it is updated to match so that the owning process
//is blisfully unaware of the update. If the region is already reserved, the
//call fails
void* reserve_physical(unsigned int physBase, unsigned int size) {

    unsigned long maxPages = maxRAM >> 12;
    unsigned int i, j, k, l, cur_page, base_page, rel_page_idx;
    unsigned char *old_mem, *new_mem;
    unsigned int page_count = 0;
    pageRange *cur_range, *new_range, *end_range;

    //Check to make sure that the physical base is at or above the 8MB mark where
    //the kernel and low memory zone ends.
    if(physBase < 0xB00000)
        return (void*)0;

    //Convert the actual allocated base to the closest page boundary (floor)
    physBase &= 0xFFFFF000;
    base_page = physBase >> 12;

    //Convert requested size to 4k pages
    if(size & 0xFFF)
        page_count += 1;

    page_count += (size >> 12);

    //Check each requested page for paging conflicts/clear and prepare space
    for(i = 0; i < page_count; i++) {

        cur_page = i + base_page;

        //If we run into a page that's already in use as reserved physical
        //space, we'll crash out
        if((pageTable[cur_page] & 0xC00) == 0xC00)
            return (void*)0;

        //Check to see if the page is marked in use and therefore needs cleanup
        if(pageTable[cur_page] & 0x800) {

            //Find the owning page tree
            for(j = 0; j < 256; j++) {

                cur_range = procTable[j].root_page;

                while(cur_range) {

                    //Check to see if the page falls within this range's space
                    if((cur_page >= cur_range->base_page) &&
                       (cur_page < (cur_range->base_page + cur_range->count))) {

                        //First, we need to truncate the current entry to
                        //the last page before the page we're going to remap
                        //and create a new range for the pages which follow
                        new_range = (pageRange*)kmalloc(sizeof(pageRange));
                        new_range->count = 1;

                        //Look for a free page
                        for(k = 0xB00; k < maxPages; k++) {

                            //Check to make sure the page is not already alocated and/or special
                            if(!(pageTable[k] & 0xC00))
                                break;
                        }

                        //Out of memory error
                        if(k == maxPages) {

                            kfree(new_range);
                            return (void*)0;
                        }

                        //Set the new page range block to map to the free
                        //memory we just found
                        new_range->base_page = k;

                        //Identity map the pages so that we can directly
                        //access it for the copy (also mark it supervisor memory)
                        map_pages(cur_page << 12, cur_page << 12, 0x1000, 3);
                        map_pages(k << 12, k << 12, 0x1000, 3);

                        //Copy the memory content
                        //(could be improved with implementation of a memcpy later on)
                        old_mem = (unsigned char*)(cur_page << 12);
                        new_mem = (unsigned char*)(k << 12);

                        for(l = 0; l < 0x1000; l++)
                            new_mem[l] = old_mem[l];

                        //Unmap the new page and mark it in use, it will be managed by the process manager
                        free_pages(k << 12, 0x1000);
                        pageTable[k] &= 0xFFFFFBFF; //Clear os-special bit
                        pageTable[k] |= 0x800; //Set in-use bit

                        //Only build a bookending range if the requested page
                        //isn't already the last page in the page range
                        if(cur_page != (cur_range->base_page + cur_range->count - 1)) {

                            end_range = (pageRange*)kmalloc(sizeof(pageRange));
                            end_range->base_page = cur_page + 1;
                            end_range->count = cur_range->count - ((cur_range->count - cur_page) + 1);
                            end_range->next = cur_range->next;
                            new_range->next = end_range;
                        } else {

                            new_range->next = cur_range->next;
                        }

                        //Point the old block at the new block
                        cur_range->count = cur_page - cur_range->base_page;
                        cur_range->next = new_range;

                        //We're done looking for the offending page
                        break;
                    }

                    //Move on to the next range block
                    cur_range = cur_range->next;
                }

                //If we found the owner, we can break out of here and move on
                if(cur_range)
                    break;
            }
        }

        //Finally, we can mark this memory as os-special and map it
        //(it is entirely possible that the above block was entered and yet
        //the offending parent was not found and remapped. In this case, we
        //just assume that the page being marked is anomalous as no page should
        //be both marked and not owned)
        //UPDATE: This is now incorrect. If we create a shared memory region, it would
        //not have an owner. This should be cleaned up, though, since we really don't
        //want to have a process accidentally free and map out a shared memory region
        //while another process is reading from it anyhow
        pageTable[cur_page] |= 0xC00; //OS-Reserved is when both os-special and in-use are set
        map_pages(cur_page << 12, cur_page << 12, 0x1000, 3);
    }

    return (void*)physBase;
}
