#ifndef BLOCK_H
#define BLOCK_H

#define BLOCKSZ 512;

typedef void (*blk_load_func)(int devId, int blknum, char* buf);

//The driver is expected to keep track of the sector currently
//held open by the given device as well as the associated buffer
//and automatically commits the stored buffer to the stored block
typedef void (*blk_stor_func)(int devId);

typedef struct block_device {
    int id;
    blk_load_func load;
    blk_stor_func store;
} block_device;

//Creates a block device structure and assigns the device a new ID
//This ID is used for getting a reference to an OS block buffer for
//the device and can also be used by the block device driver to
//keep track of what device is being referenced during an operation
void blk_device_new(block_device* newDev);

#endif //BLOCK_H
