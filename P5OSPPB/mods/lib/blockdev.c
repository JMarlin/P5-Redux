#include "../include/p5.h"
#include "../include/registrar.h"
#include "../include/blockdev.h"

unsigned int vfs_pid;
message temp_msg;

int registerAsBlockDriver() {

    //Find the VFS server
	postMessage(REGISTRAR_PID, REG_LOOKUP, SVC_VFS);
    getMessageFrom(&temp_msg, REGISTRAR_PID, REG_PID);
    vfs_pid = temp_msg.payload;

    if(!vfs_pid)
        return 0;

    //Send a register message
    postMessage(vfs_pid, VFS_REGISTER_BLOCK, 0);
    getMessageFrom(&temp_msg, vfs_pid, VFS_REGISTER_BLOCK);

    return temp_msg.payload;
}

int enumerateDevices(unsigned int driver_pid) {

    postMessage(driver_pid, BLOCKDEV_ENUMERATE, 0);
    getMessageFrom(&temp_msg, driver_pid, BLOCKDEV_ENUMERATE);

    return temp_msg.payload;
}

int getDeviceBlockSize(unsigned int driver_pid, unsigned int devnum) {

    postMessage(driver_pid, BLOCKDEV_GET_BLOCK_SIZE, devnum);
    getMessageFrom(&temp_msg, driver_pid, BLOCKDEV_GET_BLOCK_SIZE);

    return temp_msg.payload;
}

int getDeviceBlockCount(unsigned int driver_pid, unsigned int devnum) {

    postMessage(driver_pid, BLOCKDEV_GET_BLOCK_COUNT, devnum);
    getMessageFrom(&temp_msg, driver_pid, BLOCKDEV_GET_BLOCK_COUNT);

    return temp_msg.payload;
}

void* initBlockDevice(unsigned int driver_pid, unsigned int devnum) {

    postMessage(driver_pid, BLOCKDEV_INIT_DEVICE, devnum);
    getMessageFrom(&temp_msg, driver_pid, BLOCKDEV_INIT_DEVICE);

    return (void*)temp_msg.payload;
}

int closeBlockDevice(unsigned int driver_pid, unsigned int devnum) {

    postMessage(driver_pid, BLOCKDEV_CLOSE_DEVICE, devnum);
    getMessageFrom(&temp_msg, driver_pid, BLOCKDEV_CLOSE_DEVICE);

    return temp_msg.payload;
}

int initBlockRead(unsigned int driver_pid, unsigned int devnum, unsigned int lba) {

    postMessage(driver_pid, BLOCKDEV_INIT_READ, devnum);
    getMessageFrom(&temp_msg, driver_pid, BLOCKDEV_INIT_READ);

    if(!temp_msg.payload)
        return 0;

    postMessage(driver_pid, BLOCKDEV_READ_LBA, devnum);
    getMessageFrom(&temp_msg, driver_pid, BLOCKDEV_READ_LBA);

    return temp_msg.payload;
}

int initBlockWrite(unsigned int driver_pid, unsigned int devnum, unsigned int lba) {

    postMessage(driver_pid, BLOCKDEV_INIT_WRITE, devnum);
    getMessageFrom(&temp_msg, driver_pid, BLOCKDEV_INIT_WRITE);

    if(!temp_msg.payload)
        return 0;

    postMessage(driver_pid, BLOCKDEV_WRITE_LBA, devnum);
    getMessageFrom(&temp_msg, driver_pid, BLOCKDEV_WRITE_LBA);

    return temp_msg.payload;
}
