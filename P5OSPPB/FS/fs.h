#ifndef FS_H
#define FS_H 

#include "../block/block.h"

#define NUL_DEV  (block_dev*)0

#define ACT_DIR_LIST    0
#define ACT_FILE_LIST   1
#define ACT_DIR_DEL     2
#define ACT_DIR_ADD     3
#define ACT_FILE_DEL    4
#define ACT_FILE_ADD    5
#define ACT_FILE_OPEN   6
#define ACT_FILE_CLOSE  7
#define ACT_FILE_WRITEB 8
#define ACT_FILE_READB  9

#define EOF -1

typedef void (*fs_func)(block_dev*, void*, void*);
typedef void (*fs_func2)(block_dev*, void*, void*, void*);

typedef struct fsdriver {
	unsigned char type;
	fs_func dir_list;
	fs_func file_list;
	fs_func dir_del;
	fs_func dir_add;
	fs_func file_del;
	fs_func file_add;
	fs_func file_open;
	fs_func file_close;
	fs_func2 file_writeb;
	fs_func file_readb;
} fsdriver;

typedef struct fs_driver_node {
    struct fs_driver_node* next;
    fsdriver* driver;
} fs_driver_node;

typedef struct attach_point {
	unsigned char type;
	block_dev* device;
    fsdriver* driver;
    char* path;
} attach_point;

typedef struct fs_attachment_node {
    struct fs_attachment_node* next;
    attach_point* attach;
} fs_attachment_node;

typedef struct FILE {
    int id;
    attach_point* volume;    
} FILE;

fs_driver_node fs_driver_root;
fs_attachment_node fs_attachment_root;

void fs_init();
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
void dir_list(unsigned char* dir, unsigned char* list);
void file_list(unsigned char* dir, unsigned char* list);
int dir_del(unsigned char* dir);
int dir_add(unsigned char* dir);
int file_del(unsigned char* file);
int file_add(unsigned char* file);
void file_open(unsigned char* dir, FILE* file);
int file_close(FILE* file);
int file_writeb(FILE* file, unsigned char data);
int file_readb(FILE* file);
char dir_exists(unsigned char* path);
void start_process(unsigned char* path);

#endif //FS_H
