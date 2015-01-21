#include "../ascii_io/ascii_o.h"

unsigned int *pageDirectory = (unsigned int*)0x200000;
unsigned int *pageTable = (unsigned int*)0x400000;

extern void loadPageDirectory(unsigned int*);
extern void enablePaging();

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

void setPageBlock(unsigned int physAddr, unsigned int virtAddr, unsigned char flags) {
        
        int i;
        unsigned int blockNumber = virtAddr >> 22;
        unsigned int curPhysAddr = physAddr;        

        for(i = 0; i < 1024; i++) {
                setPage(blockNumber, i, curPhysAddr, flags);
                curPhysAddr += 0x1000;
        }

        pageDirectory[blockNumber] = (((unsigned int)pageTable + (blockNumber * 1024)) & 0xFFFFF000) | flags;                

}

//NOTE: Map region requires 4mb-aligned inputs. If the addresses
//given are not 4mb-aligned, they WILL be mangled
//size is given in units of 4mb 
void mapRegion(unsigned int physBase, unsigned int virtBase, unsigned int size, unsigned char flags) {

        int curVirt, curPhys, i;

        for(curVirt = virtBase, curPhys = physBase, i = 0; i < size; curVirt += 0x4000000, curPhys += 0x400000, i++) {
                prints("\nMapping block ");
                printHexDword(curPhys);
                prints(" -> ");
                printHexDword(curVirt);
                prints("\n");
                setPageBlock(curPhys, curVirt, flags);
        } 
}

void initMMU() {

        prints("\nInit page directory...");
        initPageDirectory();
        
        //map one 1:1 8mb region at the start of memory for reserved kernel
        //space
        prints("Done\nMap first 8Mb...");
        mapRegion(0x00000000, 0x00000000, 2, 3); //Flags: supervisor ram, R/W enable, page present 

        prints("Done\nLoading the page directory...");
        loadPageDirectory(pageDirectory);
        
        prints("Done\nEnabling paging...");
        enablePaging();        
        prints("Done\n");

}
