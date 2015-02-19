#include "memory.h"
#include "..\ascii_io\ascii_o.h"
#include "..\core\global.h"


extern long pkgoffset;
unsigned long maxRAM = 0x002FFFFF;
memblock rootBlock;


void testRAM() {

    int i = 0x100000;
    unsigned char prev;
    unsigned char* sysram = (unsigned char*)0;

    while(1) {
        prev = sysram[i];
        sysram[i] = ~prev;

        if(sysram[i] == prev)
            break;    

        sysram[i] = prev;
        printHexDword(i);
        prints("\n");
        i++;      
    }        

    maxRAM = i;
}


void printChain() {
 
    memblock* nextBlock = &rootBlock;

    prints("Current allocated memory:\n");

    while(1) {
        prints("   Start: 0x");
        printHexDword((unsigned long)nextBlock->base);
        prints(", Size: 0x");
        printHexDword(nextBlock->size);
        prints("\n");
                
        if(nextBlock->next)
            nextBlock = nextBlock->next;
        else
            break;
    }      
}        


void init_memory() {

    void* ram_a;
    void* ram_b;

    //Start of kernel
    rootBlock.base = (void*)0x100000;
    rootBlock.size = 0x1029A0;
    rootBlock.next = (memblock*)0;

    /*
    //testRAM();
    prints("Top of RAM: 0x");
    printHexDword(maxRAM);
    prints("\nAllocating 1k of RAM to ram_a.\n");
    ram_a = kmalloc(1024);
    
    if(ram_a == (void*)0) {
        prints("Allocation failed.\n");
        return;
    }

    //printChain();
    prints("Allocating 1k of RAM to ram_b.\n");
    ram_b = kmalloc(1024);

    if(ram_b == (void*)0) {
        prints("Allocation failed.\n");
        return;
    }

    //printChain();
    prints("Freeing ram_a.\n");
    ram_a = kfree(ram_a);
    
    if(ram_a != (void*)0) {
        prints("Free failed.\n");
        return;
    }

    //printChain();
    prints("Freeing ram_b.\n");
    ram_b = kfree(ram_b);
 
    if(ram_b != (void*)0) {
        prints("Free failed.\n");
        return;
    }

    //printChain();         
    */    
}


memblock* getMBTail() {
        
    memblock* nextBlock = &rootBlock;
        
    while(nextBlock->next)
        nextBlock = nextBlock->next;

    return nextBlock;
}

memblock* nextMBByAddress(void* baseAddr) {
        
    memblock* nextBlock = &rootBlock;
    memblock* returnBlock;
    void* nextBase = (void*)maxRAM;
    int blockFound = 0;

    while(1) {
        DEBUG("Checking address "); DEBUG_HD((unsigned long)baseAddr);
        DEBUG(" against block "); DEBUG_HD((unsigned long)nextBlock->base);
        DEBUG("-"); DEBUG_HD((unsigned long)nextBlock->base + nextBlock->size);
        DEBUG("...");
        
        if(nextBlock->base >= baseAddr && nextBlock->base < nextBase) {
            returnBlock = nextBlock;
            nextBase = returnBlock->base;
            blockFound = 1;
            DEBUG("match.\n");
        } else {
            DEBUG("no match.\n");
        }

        if(nextBlock->next)
            nextBlock = nextBlock->next;
        else
            break;
    }

    if(blockFound)
        return returnBlock;
    else
        return (memblock*)0;
}


int MBCollision(void* base, unsigned long size) {
        
    unsigned int ibase = (unsigned int)base;
    unsigned int isize = (unsigned int)size;
    unsigned int nbase, nsize;
    memblock* nextBlock = &rootBlock;

    while(1) {
        nbase = (unsigned int)nextBlock->base;
        nsize = (unsigned int)nextBlock->size;
        if((ibase >= nbase && ibase < nbase + nsize) ||
           (ibase + isize >= nbase && ibase + isize < nbase + nsize) ||
           (ibase <= nbase && ibase + isize > nbase + nsize) ||
           (ibase + isize > maxRAM))
                return 1;

        if(nextBlock->next)
            nextBlock = nextBlock->next;
        else
            return 0;
    }
}


void* kmalloc(unsigned long size) {
        
    memblock* tailBlock = getMBTail();
    memblock* nextBlock;
    memblock* newBlock;

    //This needs to be fixed when we start playing with
    //actual packages as allocations will clobber the packages
    void* rambase = (void*)(0x002029A0); //User RAM starts at 1MB

    while(1) {
                
        if(!MBCollision(rambase, size + sizeof(memblock)))
            break;

        if((nextBlock = nextMBByAddress(rambase)) == (memblock*)0)
            return (void*)0;

        rambase = nextBlock->base + nextBlock->size;
    }
                
    tailBlock->next = (memblock*)rambase;
    newBlock = tailBlock->next;
    newBlock->next = (memblock*)0;
    newBlock->base = rambase;
    newBlock->size = size + sizeof(memblock);
    return rambase + sizeof(memblock);         
}


void* kfree(void* base) {
                       
    memblock* nextBlock = &rootBlock;
    memblock* prevBlock = &rootBlock;
    void* realBase = base - sizeof(memblock);

    DEBUG("Free base: "); DEBUG_HD((unsigned long)base);
    DEBUG("\nReal base: "); DEBUG_HD((unsigned long)realBase);
    DEBUG("\n");

    while(1) {
        DEBUG("   Matches ");
        DEBUG_HD((unsigned long)nextBlock->base);
        DEBUG("?...");
        
        if(nextBlock->base == realBase) {
            DEBUG("yes\n");
            prevBlock->next = nextBlock->next;                                
            return (void*)0;
        } else {
            DEBUG("no\n");
        }
        
        if(nextBlock->next) {
            prevBlock = nextBlock;
            nextBlock = nextBlock->next;
        } else {
            return base;
        }
    }        
}
