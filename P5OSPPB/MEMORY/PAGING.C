#include "../ascii_io/ascii_o.h"
#include "../core/global.h"


extern void loadPageDirectory(unsigned int*);
extern void enablePaging();
unsigned int *pageDirectory = (unsigned int*)0x200000;
unsigned int *pageTable = (unsigned int*)0x700000;
unsigned int *pageMap = (unsigned int*)0x300000;


void clear_page_map() {

    int i;
    
    for(i = 0; i < 0x100000; i++)
        pageMap[i] = 0x0;
}


void init_page_system() {
    
    int i, j;        

    clear_page_map();
    
    //Clear the 1,024 page directory entries
    for(i = 0; i < 1024; i++) {
    
        //Set to global access, write-enabled,
        //page present
        pageDirectory[i] = 0x00000007 + (i * 0x1000);
        
        for(j = 0; j < 1024; j++) {
            
            //Same flags, but page NOT present
            pageTable[(i * 1024) + j] = 0x00000006;
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
        pageTable[i] &= 0xFFFFFFFE;
        pageTable[i] |= 0x00000004;
    }
}


void map_pages(unsigned int physBase, unsigned int virtBase, unsigned int size, unsigned short flags) {

    int i = physBase >> 12;
    int max = i + (size >> 12);
        
    for( ; i < max; i++, virtBase += 0x1000)    
        pageTable[i] = (virtBase & 0xFFFFF000) | (flags & 0xFFF);
}


void page_proc_free(void* procBase) {

    void* trackPtr, oldTrk;

    if(procBase) {
                    
        trackPtr = procBase;
        
        while(trackPtr) {
                    
            free_pages(trackPtr, 1);
            oldTrk = trackPtr;
            trackPtr = (void*)pageMap[(unsigned int)trackPtr >> 12];
            pageMap[(unsigned int)oldTrk >> 12] = 0x0;
        }
    }
}


//Allocate a chain of pages to a contiguous process space
void* page_proc_map(unsigned int procBase, unsigned int pageCount, char protect) {

    int j, i, lastPage;
    void* retPtr;
    
    for(j = 0; j < count; j++) {
    
        for(i = 0x100; i < 0x100000; i++) {
            
            if(!pageMap[i])
                break;
        }
        
        if(i == 0x100000) {
            
            //Unallocate everything we just allocated
            page_proc_free(retPtr);            
            return (void*)0x0;
        }
                            
        if(j = 0) {
        
            lastPage = i;
            retPtr = (void*)(i * 0x1000);
        } else {          
        
            pageMap[lastPage] = (void*)(i * 0x1000);
        }
        
        mapPages(i * 0x1000, procBase + (j * 0x1000), 0x1000, protect ? 3 : 7);
    }    
    
    return retPtr;
}


//Find the last page allocator in the chain
void* page_proc_last(unsigned int procBase) {

    void* trackPtr, oldTrk;

    if(procBase) {
                    
        trackPtr = procBase;
        
        while(pageMap[(unsigned int)trackPtr >> 12])                    
            trackPtr = (void*)pageMap[(unsigned int)trackPtr >> 12];
        
        return trackPtr;
    }
    
    return (void*)0x0;
}


void init_mmu() {

    //prints("\nInit page directory...");
    init_page_system();
        
    //map one 1:1 8mb region at the start of memory for reserved kernel
    //space
    DEBUG("Done\nMap first 8Mb...");
    
    //Map the first 1MB of pages as usr for v86 code
    map_pages(0x00000000, 0x00000000, 0x00100000, 7); 
        
    //Force enter the kernel's page mapping
    page_proc_map(0x00100000, 0xA00, 1);
    
    //DEBUG("Done\nMap user's 4Mb...");
    //Flags: user ram, R/W enable, page present
    //mapRegion(0x00800000, 0x00800000, 1, 7);
    DEBUG("Done\nLoading the page directory...");
    loadPageDirectory(pageDirectory);
    DEBUG("Done\nEnabling paging...");
    enablePaging();        
    DEBUG("Done\n");
}


//Toggle the 
int deactivate_proc_paging(process* proc) {
    
    
}


//Append a new page to the end of the process's allocated virtual space
int proc_add_page(process* proc) {

    
}
