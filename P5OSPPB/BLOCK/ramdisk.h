#ifndef RAMDISK_H
#define RAMDISK_H

#include "block.h"

typedef struct ram_disk {
    unsigned int id;
    unsigned int base;
    unsigned int size;
    unsigned int loadedBlk;
    char* destBuf;
} ram_disk;

typedef struct ramd_node {
    struct ramd_node* next;
    ram_disk* device;
} ramd_node;

void blk_ram_new(block_device* dev, int startAddr, int size);
void ramd_load(int devId, int blknum, char* buf);
void ramd_store(int devId, int blknum, char* buf);

#endif //RAMDISK_H
