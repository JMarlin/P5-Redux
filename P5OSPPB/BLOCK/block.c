#include "block.h"
#include "../memory/memory.h"


unsigned int blk_dev_count = 0;


void blk_device_new(block_dev* newDev) {
    
    if(!(newDev = (block_dev*)kmalloc(sizeof(block_device)))) {
    
        newDev = (block_dev*)0;
        return;
    }
    
    newDev->id = ++blk_dev_count;
}
