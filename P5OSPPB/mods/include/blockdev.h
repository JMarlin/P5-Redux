#ifndef BLOCKDEV_H
#define BLOCKDEV_H

#include "vfs.h"

#define BLOCKDEV_MSG_CLASS ((unsigned int)0x00800000)

#define BLOCKDEV_ENUMERATE       (BLOCKDEV_MSG_CLASS | 0x1)
#define BLOCKDEV_GET_BLOCK_SIZE  (BLOCKDEV_MSG_CLASS | 0x2)
#define BLOCKDEV_GET_BLOCK_COUNT (BLOCKDEV_MSG_CLASS | 0x3)
#define BLOCKDEV_INIT_DEVICE     (BLOCKDEV_MSG_CLASS | 0x4)
#define BLOCKDEV_CLOSE_DEVICE    (BLOCKDEV_MSG_CLASS | 0x5)
#define BLOCKDEV_INIT_READ       (BLOCKDEV_MSG_CLASS | 0x6)
#define BLOCKDEV_READ_LBA        (BLOCKDEV_MSG_CLASS | 0x7)
#define BLOCKDEV_INIT_WRITE      (BLOCKDEV_MSG_CLASS | 0x8)
#define BLOCKDEV_WRITE_LBA       (BLOCKDEV_MSG_CLASS | 0x9)

//This is the interface by which the FS driver and VFS will query the block devs
//Ask the driver to scan for devices and return the count 
int registerAsBlockDriver();
int enumerateDevices(unsigned int driver_pid);
int getDeviceBlockSize(unsigned int driver_pid, unsigned int devnum);
int getDeviceBlockCount(unsigned int driver_pid, unsigned int devnum);
void* initBlockDevice(unsigned int driver_pid, unsigned int devnum);
int closeBlockDevice(unsigned int driver_pid, unsigned int devnum);
int initBlockRead(unsigned int driver_pid, unsigned int devnum, unsigned int lba);
int initBlockWrite(unsigned int driver_pid, unsigned int devnum, unsigned int lba);

#endif //BLOCKDEV_H
