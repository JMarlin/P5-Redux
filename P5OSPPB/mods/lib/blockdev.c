#include "../include/p5.h"
#include "../include/registrar.h"
#include "../include/blockdev.h"

unsigned int vfs_pid;

int registerAsBlockDriver() {

    //Find the VFS server
	postMessage(REGISTRAR_PID, REG_LOOKUP, SVC_VFS);
    getMessageFrom(REGISTRAR_PID, REG_LOOKUP, &temp_msg);
    vfs_pid = temp_msg.payload;

    if(!vfs_pid)
        return 0;

    //Send a register message
    postMessage(vfs_pid, VFS_REGISTER_BLOCK, 0);
    getMessageFrom(vfs_pid, VFS_REGISTER_BLOCK, &temp_msg)

    return temp_msg.payload;
}

int enumerateDevices(unsigned int driver_pid) {

    postMessage(driver_pid, BLOCKDEV_ENUMERATE, 0);
    getMessageFrom(driver_pid, BLOCKDEV_ENUMERATE, &temp_msg);

    return temp_msg.payload;
}

int getDeviceBlockSize(unsigned int driver_pid, unsigned int devnum) {

    postMessage(driver_pid, BLOCKDEV_GET_BLOCK_SIZE, devnum);
    getMessageFrom(driver_pid, BLOCKDEV_GET_BLOCK_SIZE, &temp_msg);

    return temp_msg.payload;
}

int getDeviceBlockCount(unsigned int driver_pid, unsigned int devnum) {

    postMessage(driver_pid, BLOCKDEV_GET_BLOCK_COUNT, devnum);
    getMessageFrom(driver_pid, BLOCKDEV_GET_BLOCK_COUNT, &temp_msg);

    return temp_msg.payload;
}

void* initBlockDevice(unsigned int driver_pid, unsigned int devnum) {

    postMessage(driver_pid, BLOCKDEV_INIT_DEVICE, devnum);
    getMessageFrom(driver_pid, BLOCKDEV_INIT_DEVICE, &temp_msg);

    return (void*)temp_msg.payload;
}

int closeBlockDevice(unsigned int driver_pid, unsigned int devnum) {

    postMessage(driver_pid, BLOCKDEV_CLOSE_DEVICE, devnum);
    getMessageFrom(driver_pid, BLOCKDEV_CLOSE_DEVICE, &temp_msg);

    return temp_msg.payload;
}

int initBlockRead(unsigned int driver_pid, unsigned int devnum, unsigned int lba) {

    postMessage(driver_pid, BLOCKDEV_INIT_READ, devnum);
    getMessageFrom(driver_pid, BLOCKDEV_INIT_READ, &temp_msg);

    if(!temp_msg.payload)
        return 0;

    postMessage(driver_pid, BLOCKDEV_READ_LBA, devnum);
    getMessageFrom(driver_pid, BLOCKDEV_READ_LBA, &temp_msg);

    return temp_msg.payload;
}

int initBlockWrite(unsigned int driver_pid, unsigned int devnum, unsigned int lba) {

    postMessage(driver_pid, BLOCKDEV_INIT_WRITE, devnum);
    getMessageFrom(driver_pid, BLOCKDEV_INIT_WRITE, &temp_msg);

    if(!temp_msg.payload)
        return 0;

    postMessage(driver_pid, BLOCKDEV_WRITE_LBA, devnum);
    getMessageFrom(driver_pid, BLOCKDEV_WRITE_LBA, &temp_msg);

    return temp_msg.payload;
}
