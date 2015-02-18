#ifndef RAMFS_H
#define RAMFS_H

#include "../block/block.h"
#include "fs.h"

#define FS_RAMFS 0

void ramfs_dir_list(block_dev* dev, void* dir, void* buf);
void ramfs_file_list(block_dev* dev, void* dir, void* buf);
void ramfs_dir_del(block_dev* dev, void* dir, void* code);
void ramfs_dir_add(block_dev* dev, void* dir, void* code);
void ramfs_file_del(block_dev* dev, void* dir, void* code);
void ramfs_file_add(block_dev* dev, void* dir, void* code);
void ramfs_file_open(block_dev* dev, void* dir, void* file);
void ramfs_file_close(block_dev* dev, void* file, void* code);
void ramfs_file_writeb(block_dev* dev, void* file, void* data, void* code);
void ramfs_file_readb(block_dev* dev, void* file, void* data);

fsdriver fs_ramfs = {
    FS_RAMFS,
    &ramfs_dir_list,
    &ramfs_file_list,
    &ramfs_dir_del,
    &ramfs_dir_add,
    &ramfs_file_del,
    &ramfs_file_add,
    &ramfs_file_open,
    &ramfs_file_close,
    &ramfs_file_writeb,
    &ramfs_file_readb
};

#endif //RAMFS_H
