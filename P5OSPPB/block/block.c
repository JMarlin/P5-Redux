#include "block.h"
#include "../memory/memory.h"


unsigned int blk_dev_count;


block_dev* blk_device_new(void) {
    
    static char blk_inited = 0;
    block_dev* newDev;

    if(blk_inited != 1) {
    
        blk_inited = 1;
        blk_dev_count = 0;
    }
    
    if(!(newDev = (block_dev*)kmalloc(sizeof(block_dev)))) {
    
        return (block_dev*)0;
    }
    
    newDev->id = ++blk_dev_count;
    return newDev;
}
