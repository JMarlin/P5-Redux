#include "../include/p5.h"
#include "../include/memory.h"
#include "../include/registrar.h"
#include "../include/fs.h"

unsigned int vfs_pid;
message temp_msg;

int registerAsFSDriver() {

    //Find the VFS server
	postMessage(REGISTRAR_PID, REG_LOOKUP, SVC_VFS);
    getMessageFrom(&temp_msg, REGISTRAR_PID, REG_PID);
    vfs_pid = temp_msg.payload;

    if(!vfs_pid)
        return 0;

    //Send a register message
    postMessage(vfs_pid, VFS_REGISTER_FS, 0);
    getMessageFrom(&temp_msg, vfs_pid, VFS_REGISTER_FS);

    return temp_msg.payload;
}

int initFSDevice(unsigned int fs_driver_pid, unsigned int block_driver_pid, unsigned int device_number, unsigned int volume_id) {

    postMessage(fs_driver_pid, FS_INIT_START, volume_id);
    getMessageFrom(&temp_msg, fs_driver_pid, FS_INIT_START);

    if(!temp_msg.payload) return temp_msg.payload;

    postMessage(fs_driver_pid, FS_INIT_BLKPID, block_driver_pid);
    getMessageFrom(&temp_msg, fs_driver_pid, FS_INIT_BLKPID);

    if(!temp_msg.payload) return temp_msg.payload;

    postMessage(fs_driver_pid, FS_INIT_DEVNUM, device_number);
    getMessageFrom(&temp_msg, fs_driver_pid, FS_INIT_DEVNUM);

    return temp_msg.payload;
}

int FSPathExists(unsigned int fs_driver_pid, unsigned int volume_id, unsigned char* path) {

    postMessage(fs_driver_pid, FS_PATH_EXISTS, volume_id);
    sendString(path, fs_driver_pid);
    getMessageFrom(&temp_msg, fs_driver_pid, FS_PATH_EXISTS);

    return temp_msg.payload;
}

File* FSOpenFile(unsigned int fs_driver_pid, unsigned int volume_id, unsigned char* path) {

    File* file = (File*)malloc(sizeof(File));

    if(!file) return (File*)0;

    file->index_ptr = 0;
    void* file_buffer; 
    unsigned int buffer_size;
    unsigned int owning_driver;

    postMessage(fs_driver_pid, FS_OPEN_FILE, volume_id);
    sendString(path, fs_driver_pid);
    getMessageFrom(&temp_msg, fs_driver_pid, FS_OPEN_FILE_ID);

    if(!temp_msg.payload) return (File*)0;

    file->file_id = temp_msg.payload;

    getMessageFrom(&temp_msg, fs_driver_pid, FS_OPEN_FILE_FSZ);

    file->file_size = temp_msg.payload;

    getMessageFrom(&temp_msg, fs_driver_pid, FS_OPEN_FILE_BSZ);

    file->buffer_size = temp_msg.payload;

    getMessageFrom(&temp_msg, fs_driver_pid, FS_OPEN_FILE_OWN);

    file->owning_driver = temp_msg.payload;

    getMessageFrom(&temp_msg, fs_driver_pid, FS_OPEN_FILE_BUF);

    file->file_buffer = (void*)temp_msg.payload;

    return file;
}

int FSCloseFile(File* file) {

    postMessage(file->owning_driver, FS_CLOSE_FILE, file->file_id);
    getMessageFrom(&temp_msg, file->owning_driver, FS_CLOSE_FILE);    

    if(!temp_msg.payload) return 0;

    free(file);

    return 1;
}

//Read file chunks into buffer on nearest chunk boundary to current file index pointer
int FSRefreshFileBuffer(File* file) {

    postMessage(file->owning_driver, FS_REFRESH_BUF, file->index_ptr);
    getMessageFrom(&temp_msg, file->owning_driver, FS_REFRESH_BUF);

    return temp_msg.payload;
}

int FSFileRead(File* file, void* buffer, unsigned int count) {

    int page_ptr, remain, read, i, total;
    unsigned char* src_bytes = (unsigned char*)file->file_buffer;
    unsigned char* dst_bytes = (unsigned char*)buffer;

    //Ensure we don't read past the end of the file 
    if(count + file->index_ptr >= file->file_size)
        count = file->file_size - 1 - file->index_ptr;

    total = 0;

    while(total < count) {

        page_ptr = file->index_ptr % file->buffer_size;

        remain = file->buffer_size - page_ptr;

        read = remain < count ? remain : count;

        //Replace this with memcpy when we have that actually written properly
        for(i = 0; i < read; i++)
            dst_bytes[total + i] = src_bytes[file->index_ptr + i];

        total += read;
        file->index_ptr += read;

        //If we read up to the end of the current buffer page, load the next
        if(read == remain) 
            FSRefreshFileBuffer(file);
    }

    return total;
}

//TODO
int FSFileWrite(File* file, void* buffer, unsigned int count) {

    return 0;
}

int FSGetFileCount(unsigned int fs_driver_pid, unsigned int volume_id, unsigned char* path) {

    postMessage(fs_driver_pid, FS_GET_FILE_COUNT, volume_id);
    sendString(path, fs_driver_pid);
    getMessageFrom(&temp_msg, fs_driver_pid, FS_GET_FILE_COUNT);

    return temp_msg.payload;
}

int FSGetNthFile(unsigned int fs_driver_pid, unsigned int volume_id, unsigned char* path, unsigned int index, FileInfo* info) {

    int len;

    postMessage(fs_driver_pid, FS_GET_NTH_FILE, volume_id);
    postMessage(fs_driver_pid, FS_GET_NTH_FILE_IDX, index);
    sendString(path, fs_driver_pid);
    getMessageFrom(&temp_msg, fs_driver_pid, FS_GET_NTH_FILE_TYPE);

    info->filetype = temp_msg.payload;

    len = getStringLength(fs_driver_pid);

    info->filename = (char*)malloc(len); //TODO: Catch errors should this malloc fail
    getString(fs_driver_pid, info->filename, len);

    return 1;
}
