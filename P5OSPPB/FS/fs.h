#ifndef FS_H
#define FS_H 

#define FS_SYSFS 0
#define NUL_DEV (block_device*)0

typedef void* (*fs_func)(block_dev*, void*);
typedef void* (*fs_func2)(block_dev*, void*, void*);

typedef struct fsdriver {
	unsigned char type;
	fs_func dir_list;
	fs_func file_list;
	fs_func dir_del;
	fs_func dir_add;
	fs_func file_del;
	fs_func file_add;
	fs_func2 file_open;
	fs_func file_close;
	fs_func2 file_writeb;
	fs_func2 file_readb;
} fsdriver;

typedef struct FILE {
    int fileId;
    int headPtr;    
} FILE;

typedef struct attach_node {
	unsigned char type;
	block_dev* device;
} attach_node;

int fs_attach(unsigned char type, block_dev* device, unsigned char* point);
int fs_detach(block_dev* device);
int fs_install_driver(fsdriver* newDriver);

//The below functions first traverse the supplied path to determine the deepest attach point in the path
// eg: if we have a device mapped at . and a device mapped at .drives.hd0 and we request the folder 
// .drives.hd0.stuff.file then the deepest attach point in that path is .drives.hd0
//After determining the responsible filesystem using this method, the corresponding attach_node is found,
//the provided path is truncated (eg: .drives.hd0.stuff.file becomes .stuff.file) and forwarded to the
//responsible corresponding fsdriver function 
//ie: called:       file_open(".drives.hd0.stuff.file")
//    find node:    hd0_node = get_node(".drives.hd0.stuff.file") 
//    fs_index:     fs_index = fs_type_index(hd0_node->type);
//    forwarded to: installed_fs[fs_index]->(*file_open)(hd0_node->device, ".stuff.file") 
unsigned char** dir_list(unsigned char* dir);
unsigned char** file_list(unsigned char* dir);
int dir_del(unsigned char* dir);
int dir_add(unsigned char* dir);
int file_del(unsigned char* file);
int file_add(unsigned char* file);
FILE* file_open(unsigned char* file);
int file_close(FILE* file);
int file_writeb(FILE* file, unsigned char data);
unsigned char file_readb(FILE* file);

#endif //FS_H
