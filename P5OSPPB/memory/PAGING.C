#include "../ascii_io/ascii_o.h"
#include "../core/global.h"


extern void loadPageDirectory(unsigned int*);
extern void enablePaging();
unsigned int *pageDirectory = (unsigned int*)0x200000;
unsigned int *pageTable = (unsigned int*)0x400000;


void initPageDirectory() {
    
    int i;        

    //Clear the 1,024 page directory entries
    for(i = 0; i < 1024; i++) {
    
        //Set to supervisor-only access, write-enabled,
        //page-not-present
        pageDirectory[i] = 0x00000002;
    }
}


void setPage(unsigned int blockNumber, unsigned int blockIndex, unsigned int physAddr, unsigned char flags) {
                
    //Set the table, don't give a shit if it already existed
    pageTable[(blockNumber * 1024) + blockIndex] = (physAddr & 0xFFFFF000) | (flags & 0xFFF);
}


void setPagesInBlock(unsigned int physBase, unsigned int blockNumber, unsigned int firstPage, unsigned int pageCount, unsigned char flags) {

    int i;
    unsigned int curPhysAddr = physBase;
    
    for(i = firstPage; i < firstPage + pageCount; curPhysAddr += 0x1000, i++) {
        setPage(blockNumber, i, curPhysAddr, flags);
    }
} 


void setPageBlock(unsigned int physAddr, unsigned int virtAddr, unsigned char flags) {
        
    int i;
    unsigned int blockNumber = virtAddr >> 22;
    unsigned int curPhysAddr = physAddr;        

    for(i = 0; i < 1024; i++) {
        
        setPage(blockNumber, i, curPhysAddr, flags);
        curPhysAddr += 0x1000;
    }

    pageDirectory[blockNumber] = ((unsigned int)(pageTable + (blockNumber * 1024)) & 0xFFFFF000) | flags;                
}


//NOTE: Map region requires 4mb-aligned inputs. If the addresses
//given are not 4mb-aligned, they WILL be mangled
//size is given in units of 4mb 
void mapRegion(unsigned int physBase, unsigned int virtBase, unsigned int size, unsigned char flags) {

    int curVirt, curPhys, i;

    for(curVirt = virtBase, curPhys = physBase, i = 0; i < size; curVirt += 0x400000, curPhys += 0x400000, i++) {
        DEBUG("\nMapping block ");
        DEBUG_HD(curPhys);
        DEBUG(" -> ");
        DEBUG_HD(curVirt);
        DEBUG("\n");
        setPageBlock(curPhys, curVirt, flags);
    } 
}


void initMMU() {

    //prints("\nInit page directory...");
    initPageDirectory();
        
    //map one 1:1 8mb region at the start of memory for reserved kernel
    //space
    DEBUG("Done\nMap first 8Mb...");
    
    //Start by making a block of user ram, then change all but the first meg of 
    //that block's pages to kernel ram so that the kernel is protected but 
    //0-1Mb is user accessible for V86 code
    //Flags: user ram, R/W enable, page present
    mapRegion(0x00000000, 0x00000000, 2, 7); 
    
    //Identity map the 12K pages in block 0 above the first MB
    //Flags: kernel ram, R/W enable, page present (this is user kernel space)
    setPagesInBlock(0x00100000, 0, 0x100, 0x300, 3);
    DEBUG("Done\nMap user's 4Mb...");
    
    //Flags: user ram, R/W enable, page present
    mapRegion(0x00800000, 0x00800000, 1, 7);
    DEBUG("Done\nLoading the page directory...");
    loadPageDirectory(pageDirectory);
    DEBUG("Done\nEnabling paging...");
    enablePaging();        
    DEBUG("Done\n");
}
