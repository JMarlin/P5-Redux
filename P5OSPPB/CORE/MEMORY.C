#include "memory.h"

extern long pkgoffset;

typedef struct memblock {
        void* base;
        unsigned long size;
        memblock* next;
} memblock;

memblock rootBlock;

int init_memory() {

        //Create the root entry for the heap tree
        rootBlock.base = (void*)0x1700; //Start of kernel
        rootBlick.size = pkgoffset;
        rootBlock.next = (memblock*)0;

        //We should also probably define some managed stack 
        //space. But whatever.

}

memblock* nextMBByAddress(void* startAddress) {
        
        memblock* currentBlock = &rootBlock;
        memblock* nextBlock = &rootBlock;
        int foundBlock = 0;

        while(1) {
                if(currentBlock->base > startAddress &&
                        currentBlock->base < nextBlock->base) {
                        nextBlock = currentBlock; 
                        foundBlock = 1;
                }
                      
                if(currentBlock->next)
                        currentBlock = currentBlock.next;
                else
                        break;
        }

        if(foundBlock)
                return nextBlock;
        else
                return (memblock*)0;

}

memblock* getTailBlock() {
        
        memblock* nextBlock = &rootBlock;

        while(nextBlock->next)
                nextBlock = nextBlock->next;

        return nextBlock;

}

int blockCollision(void* base, long size) {
        
        memblock* currentBlock = &rootBlock;

        while(1) {

                if((base >= currentBlock->base &&
                    base <= currentBlock->base + currentBlock->size) ||
                   (base + size >= currentBlock->base &&
                    base + size <= currentBlock->base + currentBlock->size) ||
                   (base <= currentBlock->base &&
                    base + size >= currentBlock->base + currentBlock->size))
                        return 1;
                
                if(currentBlock->next) 
                        currentBlock = currentBlock->next;
                else
                        break;
        }        

        return 0;

}

void* kmalloc(int size) {
        
        memblock* nextBlock = &rootBlock;
        memblock* tailBlock = getTailBlock();
        void* rambase = (void*)0x100000; //User RAM starts at 1MB

        //Find the next block after rambase
        while(1) {
                            
                //If there's no collision here, this range is good.
                if(!blockCollision(rambase, size + sizeof(memblock)))
                        break;
                
                //Otherwise, find the next highest memblock 
                if((nextBlock = nextMBByAddress(rambase)) == (memblock*)0)
                        return (void*)0;                
                
                //And move the range to the end of that block
                rambase = nextBlock->base + nextBlock->size + 1;
                        
        }

        //Place the new memblock header at the top of the new memory
        tailBlock->next = (memblock*)rambase;
        tailBlock->next->size = size + sizeof(memblock);
        tailBlock->next->base = rambase;
        tailBlock->next->next = (memblock*)0; //Thus making the new block 
                                              //the new tail                      

        return rambase + sizeof(memblock);        

}

void kfree(void* base) {

        memblock* nextBlock = &rootBlock;
        memblock* prevBlock = &rootBlock;        
        void* realBase = base - sizeof(memblock);


        while(1) { 
                if(nextBlock->base == realBase) {

                        //Cut the block out of the list
                        prevBlock->next = nextBlock->next;

                        //Void the pointer
                        base = (void*)0;

                        break;
                }

                if(!nextBlock->next) break;
                prevBlock = nextBlock;
                nextBlock = nextBlock->next;       
        }

}
