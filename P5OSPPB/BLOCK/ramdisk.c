#include "ramdisk.h"
#include "block.h"


ramd_node ramd_node_root = {
    (ramd_node*)0,
    (ram_disk*)0
};


ram_disk* disk_by_id(int devId) {

    ramd_node currentNode = &ramd_node_root;
    
    if(!(currentNode->device))
        return (ram_disk*)0;
        
    while(currentNode) {
        
        if(currentNode->device->id == devId) 
            return currentNode->device;
    }
    
    return (ram_disk*)0;
}


void ramd_load(int devId, int blknum, char* buf) {
    
    ram_disk* disk;
    
    if(!(disk = disk_by_id(devId))) {
        
        buf = (char*)0;
        return;
    }
    
    if(blknum * BLOCKSZ > disk->size) {
        
        buf = (char*)0;
        return;
    }
    
    buf = (char*)(disk->base + BLOCKSZ * blknum);
}


void ramd_store(int devId) {

    //Since we're writing directly to RAM, there's nothing we
    //need to do to commit the buffer
    return;
}


int blk_ram_new(block_device* dev, int startAddr, int size) {

    ramd_node currentNode = &ramd_node_root;
    ramd_node* newNode;
    ram_disk* newDisk;
       
    if(!(newDisk = (ram_disk*)kmalloc(sizeof(ram_disk)))) 
        return 0;
    
    newDisk->base = startAddr;
    newDisk->size = size;
    newDisk->id = dev->id;
    dev->load = &ramd_load;
    dev->store = &ramd_store;
    
    if(!(currentNode->device)) {
    
        currentNode->device = newDisk;
        return 1;
    }
    
    if(!(newNode = (ramd_node*)kmalloc(sizeof(ramd_node)))) {
 
        kfree(newNode); 
        return 0;
    }
    
    newNode->device = newDisk;
    newNode->next = (ramd_node*)0;
    
    while(currentNode->next)
        currentNode = currentNode->next;
        
    currentNode->next = newNode;
    return 1;
}
