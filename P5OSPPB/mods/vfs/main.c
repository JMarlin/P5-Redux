#include "../include/p5.h"
#include "../include/memory.h"
#include "../include/registrar.h"
#include "../include/blockdev.h"
#include "../include/fs.h"

//In the VFS, we'll keep a list of device structs. 
//The device struct will associate the system-wide 
//device ID with a block driver PID, the block device 
//number in that device driver, a pointer to the
//transfer buffer and other elements allowing access 
//of the device 

#define VOLUME_MOUNTED 0x01

//Structs for storing mounted filesystems
typedef struct Volume_struct {
    unsigned int flags;
    unsigned int volume_id;
    unsigned int block_driver_pid;
    unsigned int block_device_number;
    void* block_buffer;
    unsigned int fs_driver_pid;
} Volume;

//Global volume list
unsigned int volume_count = 0;
Volume** volume_list = 0;

//Global registered block driver list
unsigned int block_driver_count = 0;
unsigned int* block_pid_list = 0;

//Global registered file system driver list
unsigned int fs_driver_count = 0;
unsigned int* fs_pid_list = 0;

Volume* Volume_new() {

    static unsigned int vid_source = 0;
    Volume** new_list;
    Volume* volume = (Volume*)malloc(sizeof(Volume));

    if(!volume)
        return volume;

    if(!volume_list)
        new_list = (Volume**)malloc(sizeof(Volume*)*(volume_count + 1));
    else
        new_list = (Volume**)realloc(volume_list, sizeof(Volume*)*(volume_count + 1));

    if(!new_list)
        return (Volume*)0;

    volume_list = new_list;
    volume_list[volume_count++] = volume;

    volume->volume_id = ++vid_source;
    volume->block_driver_pid = 0;
    volume->block_device_number = 0;
    volume->block_buffer = 0;
    volume->fs_driver_pid = 0;

    return volume;
}

int getVolumePositionById(unsigned int volume_id) {

    int i;

    for(i = 0; i < volume_count; i++)
        if(volume_list[i]->volume_id == volume_id)
            return i;
    
    return -1;
}

Volume* getVolumeById(unsigned int volume_id) {

    int list_idx = getVolumePositionById(volume_id);

    if(list_idx < 0)
        return (Volume*)0;

    return volume_list[list_idx];
}

int requestCloseVolume(Volume* volume) {

    //Call driver to shut down device 
    //This is async, so we just make the request here and then
    //defer any housekeeping until the driver sends us a 
    //message informing us that the device is closed 
    return closeBlockDevice(volume->block_driver_pid, volume->block_device_number);

    //When that returns in the message loop, we should delete the 
    //item from the list and collapse the list 
    //but that can be deferred to later
}

int registerNewBlockDriver(unsigned int driver_pid) {

    unsigned int* new_list;

    if(!block_pid_list)
        new_list = (unsigned int*)malloc(sizeof(unsigned int)*(block_driver_count + 1));
    else
        new_list = (unsigned int*)realloc(block_pid_list, sizeof(unsigned int)*(block_driver_count + 1));

    if(!new_list)
        return 0;

    block_pid_list = new_list;
    block_pid_list[block_driver_count++] = driver_pid;

    return 1;
}

int registerNewFSDriver(unsigned int driver_pid) {

    unsigned int* new_list;

    if(!fs_pid_list)
        new_list = (unsigned int*)malloc(sizeof(unsigned int)*(fs_driver_count + 1));
    else
        new_list = (unsigned int*)realloc(fs_pid_list, sizeof(unsigned int)*(fs_driver_count + 1));

    if(!new_list)
        return 0;

    fs_pid_list = new_list;
    fs_pid_list[fs_driver_count++] = driver_pid;

    return 1;
}

int Volume_mount(Volume* volume) {

    int i;

    if(volume->flags & VOLUME_MOUNTED)
        return 1; //No need to mount again 

    for(i = 0; i <  fs_driver_count; i++)
        if(initFSDevice(fs_pid_list[i], volume->block_driver_pid, volume->block_device_number, volume->volume_id)) {

            volume->flags |= VOLUME_MOUNTED;
            volume->fs_driver_pid = fs_pid_list[i];

            return 1;
        }

    return 0;
}

void mountScan() {

    int i;

    for(i = 0; i < volume_count; i++)
        if(!(volume_list[i]->flags & VOLUME_MOUNTED))
            Volume_mount(volume_list[i]);
}

Volume* Volume_fromDevice(unsigned int driver_pid, unsigned int device_number) {

    Volume* volume = 0;
    void* new_buffer = initBlockDevice(driver_pid, device_number);

    if(!new_buffer)
        return volume;

    if(!(volume = Volume_new())) {

        //Again, keep in mind that this is async
        closeBlockDevice(driver_pid, device_number);

        return volume;
    }

    volume->flags = 0;
    volume->block_driver_pid = driver_pid;
    volume->block_device_number = device_number;
    volume->block_buffer = new_buffer;

    //This may or may not succeed, but for the purposes of this function we don't really care
    Volume_mount(volume);

    return volume;
}

void main(void) {

    unsigned char* disk_buffer;
    message temp_msg;
    unsigned int parent_pid;

	//Get the 'here's my pid' message from init
    getMessage(&temp_msg);
    parent_pid = temp_msg.source;

    prints("[VFS] Starting VFS process...\n");

    prints("[VFS] Registering as VFS service with the registrar...");
    postMessage(REGISTRAR_PID, REG_REGISTER, SVC_VFS);
    getMessage(&temp_msg);

    if(!temp_msg.payload) {

        prints("Failed\n");
        postMessage(parent_pid, 0, 0); //Tell the parent we're failed
        while(1); //Hang the service (would be a terminate once we have that working properly)
    } 

    prints("Success\n");
    postMessage(parent_pid, 0, 1); //Tell the parent we started up OK

    //Enter the message loop
    while(1) {

        getMessage(&temp_msg); //Putting this in a loop shouldn't actually matter now since the OS sleeps procs waiting for a message

        switch(temp_msg.command) {

            case VFS_REGISTER_BLOCK:
                //DEBUG 
                prints("\n[VFS] registering new block device server at PID 0x");
                printHexDword(temp_msg.source);
                prints("...");
                
                if(registerNewBlockDriver(temp_msg.source)) {

                    prints("Success\n"); //DEBUG
                    postMessage(temp_msg.source, VFS_REGISTER_BLOCK, 1);

                    //If we succeed here, we should request the number of 
                    //devices, then initialize each device in turn and 
                    //attempt to mount it with each registered FS driver
                    //either stopping when a successful mount occurs or,
                    //should we hit the end of the list without a good
                    //mount, creating a volume for the drive with a 
                    //zero fs driver PID and a cleared mount flag to 
                    //indicate an undetectable volume 
                    int i, 
                        devices = enumerateDevices(block_pid_list[block_driver_count - 1]);
                    
                    for(i = 0; i < devices; i++) //Note: device numbers are 1-indexed
                        Volume_fromDevice(block_pid_list[block_driver_count - 1], i + 1);

                } else {

                    prints("Failure\n"); //DEBUG
                    postMessage(temp_msg.source, VFS_REGISTER_BLOCK, 0);
                }
            break;

            case VFS_REGISTER_FS:
                //DEBUG 
                prints("\n[VFS] registering new filesystem server at PID 0x");
                printHexDword(temp_msg.source);
                prints("...");
                
                if(registerNewFSDriver(temp_msg.source)) {

                    prints("Success\n"); //DEBUG
                    postMessage(temp_msg.source, VFS_REGISTER_FS, 1);

                    //If we succeed here, we should iterate through the list of
                    //volumes and, should we find an entry that lacks a mounted
                    //flag or an FS PID, attempt to mount the volume with the 
                    //newly registered FS driver
                    mountScan();
                } else {

                    prints("Failure\n"); //DEBUG
                    postMessage(temp_msg.source, VFS_REGISTER_FS, 0);
                }
            break;

            default:
            break;
        }
    }
}
