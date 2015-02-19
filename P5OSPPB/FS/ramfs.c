#include "../block/block.h"
#include "ramfs.h"
#include "../ascii_io/ascii_i.h"
#include "../memory/memory.h"


unsigned char blk_buf[BLOCKSZ];
fsdriver fs_ramfs;


fsdriver* get_ramfs_driver() {

    fs_ramfs.type = FS_RAMFS;
    fs_ramfs.dir_list = &ramfs_dir_list;
    fs_ramfs.file_list = &ramfs_file_list;
    fs_ramfs.dir_del = &ramfs_dir_del;
    fs_ramfs.dir_add = &ramfs_dir_add;
    fs_ramfs.file_del = &ramfs_file_del;
    fs_ramfs.file_add = &ramfs_file_add;
    fs_ramfs.file_open = &ramfs_file_open;
    fs_ramfs.file_close = &ramfs_file_close;
    fs_ramfs.file_writeb = &ramfs_file_writeb;
    fs_ramfs.file_readb = &ramfs_file_readb;
    return &fs_ramfs;
}


unsigned char block_linear_read(block_dev* dev, int index) {

    static int loadedBlock = -1;
    int calcBlock = (index / BLOCKSZ);
    int calcIndex = (index % BLOCKSZ);
    
    if(calcBlock != loadedBlock) {
    
        loadedBlock = calcBlock;
        dev->load(dev->id, loadedBlock, blk_buf);
    }
    
    return blk_buf[calcIndex];
}


//Returns directories delimited by colons
//An empty string is a failed operation
//A string with a single colon is an empty listing 
void ramfs_dir_list(block_dev* dev, void* dir, void* buf) {

    //Really need a way to make sure we don't 
    //overflow the user's buffer, but fuck it for now
    char* dirlist = buf;
    
    if(strcmp(dir, ":")) {
        dirlist[0] = ':';
        dirlist[1] = 0;
    } else {
        dirlist[0] = 0;
    }
}


//Returns directories delimited by colons
//An empty string is a failed operation
//A string with a single colon is an empty listing 
void ramfs_file_list(block_dev* dev, void* vdir, void* buf) {

    //Really need a way to make sure we don't 
    //overflow the user's buffer, but fuck it for now
    char* dirlist = (char*)buf;
    char* dir = (char*)vdir;
    int i, offset, count, listsz;
    char strlen;
    
    prints("\nCalled file listing for subdir '");
    prints(dir);
    prints("' in ramfs filesystem on device #");
    printHexDword(dev->id);
    prints("\n");
    listsz = 0;
       
    //ramfs doesn't have directories beneath root
    if(!strcmp(dir, ":")) {    
        
        dirlist[0] = 0;
        return;
    }
    
    offset = 0;
    count = block_linear_read(dev, offset++);
    count |= block_linear_read(dev, offset++) << 8;
    count |= block_linear_read(dev, offset++) << 16;
    count |= block_linear_read(dev, offset++) << 24;
    
    if(count) {

        while(count) {
            
            //Skip the payload pointers
            offset += 8;
            strlen = block_linear_read(dev, offset++);
            
            for(i = 0; i < strlen; listsz++, i++)
                dirlist[listsz] = block_linear_read(dev, offset++);
                            
            count--;
            if(count) dirlist[listsz++] = ':';
        }
    } else {
        
        dirlist[listsz++] = ':';
    }
    
    dirlist[listsz] = 0;
}


void ramfs_dir_del(block_dev* dev, void* dir, void* code) {

    //You can't do this in ramfs
    ((int*)code)[0] = 0;
}


void ramfs_dir_add(block_dev* dev, void* dir, void* code) {

    //Also can't do this
    ((int*)code)[0] = 0;
}


void ramfs_file_del(block_dev* dev, void* dir, void* code) {

    //Can't do this either
    ((int*)code)[0] = 0;
}


void ramfs_file_add(block_dev* dev, void* dir, void* code) {

    //Nope, this is off limits as well
    ((int*)code)[0] = 0;
}


void ramfs_file_open(block_dev* dev, void* dir, void* file) {

    //This works, but we should probably figure out how file handles work
}


void ramfs_file_close(block_dev* dev, void* file, void* code) {

    //I guess this should pretty much just free the file handle struct
}


void ramfs_file_writeb(block_dev* dev, void* file, void* data, void* code) {

    //You want to write into a read only fs?
    //((int*)code)[0] = 0;
}


void ramfs_file_readb(block_dev* dev, void* file, void* data) {

    //Again, this kind of depends on how the file handle works
}
