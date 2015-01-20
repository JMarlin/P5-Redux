#include "memory.h"
#include "..\ascii_io\ascii_o.h"

extern long pkgoffset;

typedef struct memblock {
        void* base;
        unsigned long size;
        struct memblock* next;
} memblock;

unsigned long maxRAM = 0x2000000;
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

        rootBlock.base = (void*)0x100000; //Start of kernel
        rootBlock.size = pkgoffset;
        rootBlock.next = (memblock*)0;

//        testRAM();
        prints("Top of RAM: 0x");
        printHexDword(maxRAM);
        prints("\n");

        prints("Allocating 1k of RAM to ram_a.\n");
        ram_a = kmalloc(1024);
        if(ram_a == (void*)0) {
                prints("Allocation failed.\n");
                return;
        }
//        printChain();

        prints("Allocating 1k of RAM to ram_b.\n");
        ram_b = kmalloc(1024);
        if(ram_b == (void*)0) {
                prints("Allocation failed.\n");
                return;
        }
//        printChain();
        
        prints("Freeing ram_a.\n");
        ram_a = kfree(ram_a);
        if(ram_a != (void*)0) {
                prints("Free failed.\n");
                return;
        }
//        printChain();

        prints("Freeing ram_b.\n");
        ram_b = kfree(ram_b);
        if(ram_b != (void*)0) {
                prints("Free failed.\n");
                return;
        }
//        printChain();                

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
//                prints("Checking address "); printHexDword((unsigned long)baseAddr);
//                prints(" against block "); printHexDword((unsigned long)nextBlock->base);
//                prints("-"); printHexDword((unsigned long)nextBlock->base + nextBlock->size);
//                prints("...");
                if(nextBlock->base >= baseAddr && nextBlock->base < nextBase) {
                        returnBlock = nextBlock;
                        nextBase = returnBlock->base;
                        blockFound = 1;
//                        prints("match.\n");
                }else{
//                        prints("no match.\n");
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
        
        memblock* nextBlock = &rootBlock;

        while(1) {
                if((base >= nextBlock->base && 
                    base < nextBlock->base + nextBlock->size) ||
                   (base + size >= nextBlock->base &&
                    base + size < nextBlock->base + nextBlock->size) ||
                   (base <= nextBlock->base &&
                    base + size > nextBlock->base + nextBlock->size) ||
                   (base + size > maxRAM))
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
        void* rambase = (void*)(0x100000 + pkgoffset); //User RAM starts at 1MB

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
//        prints("Free base: "); printHexDword((unsigned long)base);
        
                
        memblock* nextBlock = &rootBlock;
        memblock* prevBlock = &rootBlock;
        void* realBase = base - sizeof(memblock);

//        prints("\nReal base: "); printHexDword((unsigned long)realBase);
//        prints("\n");

        while(1) {
//                prints("   Matches ");
//                printHexDword((unsigned long)nextBlock->base);
//                prints("?...");
                if(nextBlock->base == realBase) {
//                        prints("yes\n");
                        prevBlock->next = nextBlock->next;                                
                        return (void*)0;
                }else{
//                        prints("no\n");
                }
        
                if(nextBlock->next) {
                        prevBlock = nextBlock;
                        nextBlock = nextBlock->next;
                } else {
                        return base;
                }
        
        }        

}
