#include "../block/block.h"
#include "ramfs.h"
#include "fs.h"
#include "../ascii_io/ascii_i.h"
#include "../memory/memory.h"
#include "../core/global.h"


unsigned char blk_buf[BLOCKSZ];
fsdriver fs_ramfs;
ramfs_file_node open_files_root;


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


ramfs_file* get_ramfile_by_id(int id) {

    ramfs_file_node* currentNode = &open_files_root;
    
    if(!currentNode->file)
        return (ramfs_file*)0;
        
    while(currentNode) {
    
        if(currentNode->file->id == id)
            return currentNode->file;
        
        currentNode = currentNode->next;
    }
    
    return (ramfs_file*)0;
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
    
    DEBUG("\nCalled file listing for subdir '");
    DEBUG(dir);
    DEBUG("' in ramfs filesystem on device #");
    DEBUG_HD(dev->id);
    DEBUG("\n");
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


//A lot of this could be consolidated into a read n-th filename function
//which could be used in both file_list and seekFile
int ramfs_seekFile(block_dev* dev, unsigned char* dir, ramfs_file* newRamFile) {

    unsigned char *seekName, *tmpName;
    int i, count, offset, strlen, fileOffset, fileSize;
        
    //Don't waste time on an empty string
    if(dir[0] == 0 || dir[1] == 0) {
    
        prints("\nPath is empty\n");
        return 0;
    }
        
    //Lop off the leading colon
    seekName = dir + 1;
    
    if(!(tmpName = (unsigned char*)kmalloc(256))) {
    
        prints("\nCouldn't allocate RAM\n");
        return 0;
    }
        
    offset = 0;
    count = block_linear_read(dev, offset++);
    count |= block_linear_read(dev, offset++) << 8;
    count |= block_linear_read(dev, offset++) << 16;
    count |= block_linear_read(dev, offset++) << 24;
    
    while(count) {
            
            //Get the payload pointers
            fileOffset = block_linear_read(dev, offset++);
            fileOffset |= block_linear_read(dev, offset++) << 8;
            fileOffset |= block_linear_read(dev, offset++) << 16;
            fileOffset |= block_linear_read(dev, offset++) << 24;
            fileSize = block_linear_read(dev, offset++);
            fileSize |= block_linear_read(dev, offset++) << 8;
            fileSize |= block_linear_read(dev, offset++) << 16;
            fileSize |= block_linear_read(dev, offset++) << 24;
            strlen = block_linear_read(dev, offset++);
            
            //For now, we're just cutting it off at the buffer
            //limit. I guess we could just call 256 chars the max
            //filename limit
            for(i = 0; i < strlen && i < 256; i++)
                tmpName[i] = block_linear_read(dev, offset++);
            
            tmpName[i] = 0;
                                                
            if(strcmp(tmpName, seekName)) {
                
                newRamFile->offset = fileOffset;
                newRamFile->length = fileSize;
                kfree((void*)tmpName);
                return 1;
            } 
            
            count--;
    }
        
    kfree((void*)tmpName);
    return 0;
}


void ramfs_file_open(block_dev* dev, void* vdir, void* vfile) {

    static char open_files_inited = 0;
    ramfs_file_node* currentNode = &open_files_root;
    unsigned char* dir = (unsigned char*)vdir;
    FILE* file = (FILE*)vfile;
    ramfs_file_node* newNode;
    ramfs_file* newRamFile;
    
    if(open_files_inited != 1) {
        
        open_files_inited = 1;
        open_files_root.next = (ramfs_file_node*)0;
        open_files_root.file = (ramfs_file*)0;
    }
            
    //prepare a ramfs_file structure
    if(!(newRamFile = (ramfs_file*)kmalloc(sizeof(ramfs_file)))) 
        return;
    
    //Map the new ramfs_file to the OS file handle
    newRamFile->id = file->id;
    
    //Look up the file
    //This populates the offset and size values
    if(!ramfs_seekFile(dev, dir, newRamFile)) {

        kfree((void*)newRamFile);
        return;
    }
    
    //Reset the new ramfs_file's index to 0
    newRamFile->index = 0;
    
    prints("\nRAM file opened, offset=0x"); 
    printHexDword(newRamFile->offset);
    prints(", size=0x");
    printHexDword(newRamFile->length);
    prints("\n");
    
    //If the root node is unpopulated, all we need
    //to do is insert the new ramfs_file
    if(!currentNode->file) {
        
        currentNode->file = newRamFile;
        return;
    }
        
    //Allocate a new node
    if(!(newNode = (ramfs_file_node*)kmalloc(sizeof(ramfs_file_node))))  {

        kfree((void*)newRamFile);
        return;
    }    
    
    //Populate it
    newNode->next = (ramfs_file_node*)0;
    newNode->file = newRamFile;
    
    //Fast-forward to the end of the list
    while(currentNode->next)
        currentNode = currentNode->next;
        
    //Install the node
    currentNode->next = newNode;
}


void ramfs_file_close(block_dev* dev, void* file, void* code) {

    //I guess this should pretty much just free the file handle struct
    //But for now, we're just not going to close it for testing of read function
    ((int*)code)[0] = 0;
}


void ramfs_file_writeb(block_dev* dev, void* file, void* data, void* code) {

    //You want to write into a read only fs?
    ((int*)code)[0] = 0;
}


void ramfs_file_readb(block_dev* dev, void* vfile, void* vdata) {

    prints("\nEntered ramfs readb\n");

    int* data = (int*)vdata;
    FILE* file = (FILE*)vfile;   
    ramfs_file* ramFile = get_ramfile_by_id(file->id);
    
    if(!ramFile) {
    
        data[0] = EOF;
        return;
    }
    
    if(ramFile->index >= ramFile->length) {
        
        data[0] = EOF;
        return;
    }    
    
    data[0] = (int)block_linear_read(dev, ramFile->offset + ramFile->index);
}
