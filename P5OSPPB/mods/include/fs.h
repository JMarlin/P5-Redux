#ifndef FS_H
#define FS_H 

#include "vfs.h"

#define FS_MSG_CLASS ((unsigned int)0x00A00000)

#define FS_INIT_START        (FS_MSG_CLASS | 0x1)
#define FS_INIT_BLKPID       (FS_MSG_CLASS | 0x2)
#define FS_INIT_DEVNUM       (FS_MSG_CLASS | 0x3)
#define FS_PATH_EXISTS       (FS_MSG_CLASS | 0x4)
#define FS_OPEN_FILE         (FS_MSG_CLASS | 0x5)
#define FS_OPEN_FILE_ID      (FS_MSG_CLASS | 0x6)
#define FS_OPEN_FILE_FSZ     (FS_MSG_CLASS | 0x7)
#define FS_OPEN_FILE_BSZ     (FS_MSG_CLASS | 0x8)
#define FS_OPEN_FILE_OWN     (FS_MSG_CLASS | 0x9)
#define FS_OPEN_FILE_BUF     (FS_MSG_CLASS | 0xA)
#define FS_CLOSE_FILE        (FS_MSG_CLASS | 0xB)
#define FS_REFRESH_BUF       (FS_MSG_CLASS | 0xC)
#define FS_GET_FILE_COUNT    (FS_MSG_CLASS | 0xD)
#define FS_GET_NTH_FILE      (FS_MSG_CLASS | 0xE)
#define FS_GET_NTH_FILE_IDX  (FS_MSG_CLASS | 0xF)
#define FS_GET_NTH_FILE_TYPE (FS_MSG_CLASS | 0x10)

typedef struct File_struct {
    unsigned int file_id;
    unsigned int file_size;
    unsigned int index_ptr;
    void* file_buffer; //Shared memory page
    unsigned int buffer_size;
    unsigned int owning_driver;
} File;

typedef struct FileInfo_struct {
    char* filename;
    unsigned char filetype;
} FileInfo;

int registerAsFSDriver();
int initFSDevice(unsigned int fs_driver_pid, unsigned int block_driver_pid, unsigned int device_number, unsigned int volume_id);
int FSPathExists(unsigned int fs_driver_pid, unsigned int volume_id, unsigned char* path);
File* FSOpenFile(unsigned int fs_driver_pid, unsigned int volume_id, unsigned char* path);
int FSCloseFile(File* file);
int FSRefreshFileBuffer(File* file); //Read file chunks into buffer on nearest chunk boundary to current file index pointer
int FSFileRead(File* file, void* buffer, unsigned int count);
int FSFileWrite(File* file, void* buffer, unsigned int count);
int FSGetFileCount(unsigned int fs_driver_pid, unsigned int volume_id, unsigned char* path);
int FSGetNthFile(unsigned int fs_driver_pid, unsigned int volume_id, unsigned char* path, unsigned int index, FileInfo* info);

#endif //FS_H