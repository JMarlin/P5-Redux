#include "../block/block.h"
#include "ramfs.h"


void* ramfs_dir_list(block_dev* dev, void* dir) {
    
    return (void*)0;
}


void* ramfs_file_list(block_dev* dev, void* dir) {

    
}


void* ramfs_dir_del(block_dev* dev, void* dir);
void* ramfs_dir_add(block_dev* dev, void* dir);
void* ramfs_file_del(block_dev* dev, void* dir);
void* ramfs_file_add(block_dev* dev, void* dir);
void* ramfs_file_open(block_dev* dev, void* dir);
void* ramfs_file_close(block_dev* dev, void* dir);
void* ramfs_file_writeb(block_dev* dev, void* file, void* data);
void* ramfs_file_readb(block_dev* dev, void* file);
