#include "block.h"
#include "../memory/memory.h"


unsigned int blk_dev_count = 0;


void blk_device_new(block_device* newDev) {
    
    if(!(newDev = (block_device*)kmalloc(sizeof(block_device)))) {
    
        newDev = (block_device*)0;
        return;
    }
    
    newDev->id = ++blk_dev_count;
}
