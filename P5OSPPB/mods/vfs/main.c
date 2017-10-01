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

unsigned int pathCmp(char* path, char* cmp) {

    while(*cmp)
        if(*(path++) != *(cmp++))
            return 0;

    return 1;
}

char* krd_base = 0;
unsigned int krd_size = 0;
char temp_buf[256] = {0};

void krd_init() {

    krd_size = *((unsigned int*)0x100005);
    krd_base = (unsigned char*)0x100005 + krd_size;
}

unsigned int krd_getFileCount() {

    return *((unsigned int*)krd_base);
}

void krd_getNthFilename(int index, char* outbuf, char* out_len) {

    char* entry_base = krd_base + 4;
    unsigned char len;
    int i;

    if(index >= krd_getFileCount()) {

        outbuf[0] = 0;
        *out_len = 0;
        return;
    }

    for(i = 0; i <= index; i++) {

        entry_base += 8;
        len = *((unsigned char*)(entry_base++));
        
        if(i < index)
            entry_base += len;
    }

    *out_len = len;

    for(i = 0; i < len; i++)
        outbuf[i] = *(entry_base++);

    outbuf[i] = 0;
}

unsigned int vfs_pathExists(char* path) {

    unsigned int vol_num;
    int file_count, i;
    char len;
    Volume* target_volume;

    //Check for root delimiter, return fail if it's not there
    if(*(path++) != ':')
        return 0;

    //The root directory always exists
    if(*path == 0)
        return 1;

    //Compare next segment to registered volume names. If there's a match and
    //    another segment following, forward to the volume's FS driver. 
    //    Otherwise, return success
    //Later, we'll get volume names from the FS drivers themselves. But for 
    //now, to start simply, the volumes are simply 0-9
    if(pathCmp(path, "vol_")) {

        path += 4;
        vol_num = (unsigned int)(*(path++) - '0');

        if((vol_num < 10) &&
           (target_volume = getVolumeById(vol_num)) &&
           (target_volume->flags & VOLUME_MOUNTED)) {

            if(*path == ':')
                return FSPathExists(target_volume->fs_driver_pid, target_volume->volume_id, path);
            
            if(*path == 0)
                return 1;
            else
                return 0;
        }

        //If we had a bad volume number or volume doesn't exist, we need to rewind
        path -= 5;        
    }

    //Use kernelfs functions to compare the segment to the list of ramdisk
    //    files. If no match, return fail. If match, return fail if not at 
    //    end of path. Otherwise return success.
    file_count = krd_getFileCount();

    for(i = 0; i < file_count; i++) {

        krd_getNthFilename(i, temp_buf, &len);
        
        if(pathCmp(path, temp_buf) && (*(path + len) == 0))
            return 1;
    }

    return 0;
}

unsigned int vfs_mountedVolumeCount() {

    int i;
    unsigned int sum = 0;

    //This would go faster if we just updated a number
    //whenever mounting or unmounting a volume
    for(i = 0; i < volume_count; i++)
        if(volume_list[i]->flags & VOLUME_MOUNTED) sum++;

    return sum;
}

unsigned int vfs_fileCount(char* path) {

    unsigned int vol_num;
    Volume* target_volume;

    //Check for root delimiter, return fail if it's not there
    if(*(path++) != ':')
        return 0;

    //If we're just querying the root path, just return the number of files
    //in the kernel ramdisk plus the number of registered volumes
    if(*path == 0)
        return krd_getFileCount() + vfs_mountedVolumeCount();

    //Compare next segment to registered volume names. If there's a match and
    //    another segment following, forward to the volume's FS driver. 
    //    Otherwise, return success
    //Later, we'll get volume names from the FS drivers themselves. But for 
    //now, to start simply, the volumes are simply 0-9
    if(pathCmp(path, "vol_")) {

        path += 4;
        vol_num = (unsigned int)(*(path++) - '0');

        if((vol_num < 10) &&
           (target_volume = getVolumeById(vol_num)) &&
           (target_volume->flags & VOLUME_MOUNTED)) {

            if(*path == ':')
                return FSGetFileCount(target_volume->fs_driver_pid, target_volume->volume_id, path);
            else
                return 0;
        }    
    }

    return 0;
}

void vfs_getNthFileInfo(char* path, unsigned int index, FileInfo* file_info) {

    unsigned int vol_num, i, vol_idx;
    char len;
    Volume* target_volume;

    file_info->filename = 0;
    file_info->filetype = 0;

    //Check for root delimiter, return fail if it's not there
    if(*(path++) != ':')
        return;

    //Listing will go volumes followed by ramdisk files
    //If the index is in the volume range, we return the generated
    //volume name and the 'folder' file type. Otherwise, we return
    //the ramdisk filename and the 'file' file type 
    if(*path == 0) {

        if(index < vfs_mountedVolumeCount()) {

            //Get indexth mounted volume
            vol_idx = 0;

            for(i = 0; i < volume_count; i++)
                if(volume_list[i]->flags & VOLUME_MOUNTED) {

                    vol_idx++;
                    if(vol_idx == index)
                        break;
                }

            //TODO: Catch the above loop not actually finding a mounted volume

            //Again, this is one of those spots where, in the future, 
            //we would be deferring to the filesystem driver to give us
            //the volume name
            file_info->filename = (char*)malloc(6);
            file_info->filename[0] = 'v';
            file_info->filename[1] = 'o';
            file_info->filename[2] = 'l';
            file_info->filename[3] = '_';
            file_info->filename[4] = (char)(volume_list[i]->volume_id & 0xFF) + '0';
            file_info->filename[5] = 0;
            file_info->filetype = 2;
        } else if(index < (vfs_mountedVolumeCount() + krd_getFileCount())) {

            krd_getNthFilename(index - vfs_mountedVolumeCount(), temp_buf, &len);

            file_info->filename = (char*)malloc(len + 1);

            for(i = 0; i < len; i++)
                file_info->filename[i] = temp_buf[i];

            file_info->filename[i] = 0;
            file_info->filetype = 1;
        }

        return; //Bad index
    }

    //Compare next segment to registered volume names. If there's a match and
    //    another segment following, forward to the volume's FS driver. 
    //    Otherwise, return success
    //Later, we'll get volume names from the FS drivers themselves. But for 
    //now, to start simply, the volumes are simply 0-9
    if(pathCmp(path, "vol_")) {

        path += 4;
        vol_num = (unsigned int)(*(path++) - '0');

        if((vol_num < 10) &&
           (target_volume = getVolumeById(vol_num)) &&
           (target_volume->flags & VOLUME_MOUNTED)) {

            if(*path == ':') 
                FSGetNthFile(target_volume->fs_driver_pid, target_volume->volume_id, path, index, file_info);

            if(*path == 0) 
                FSGetNthFile(target_volume->fs_driver_pid, target_volume->volume_id, ":", index, file_info);
        }    
    }
}

void main(void) {

    unsigned char* disk_buffer;
    message temp_msg;
    unsigned int parent_pid;
    unsigned int call_client_pid;  
    unsigned int call_volume_id;
    unsigned int call_path_string_length; 
    unsigned int call_index;
    FileInfo file_info;
    char* call_path_string;

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

    //Set up the references to the kernel ramdisk
    krd_init();

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

            case FS_PATH_EXISTS:
                 call_client_pid = temp_msg.source;
                 call_volume_id = temp_msg.payload;
                 call_path_string_length = getStringLength(call_client_pid); 
                 call_path_string = (char*)malloc(call_path_string_length);
                 getString(call_client_pid, call_path_string, call_path_string_length);
                 postMessage(call_client_pid, FS_PATH_EXISTS, vfs_pathExists(call_path_string));
                 free(call_path_string);
            break;

            case FS_GET_FILE_COUNT:
                call_client_pid = temp_msg.source;
                call_volume_id = temp_msg.payload;
                call_path_string_length = getStringLength(call_client_pid); 
                call_path_string = (char*)malloc(call_path_string_length);
                getString(call_client_pid, call_path_string, call_path_string_length);
                postMessage(call_client_pid, FS_GET_FILE_COUNT, vfs_fileCount(call_path_string));
                free(call_path_string);
            break;

            case FS_GET_NTH_FILE:
                call_client_pid = temp_msg.source;
                call_volume_id = temp_msg.payload;
                getMessageFrom(&temp_msg, call_client_pid, FS_GET_NTH_FILE_IDX);
                call_index = temp_msg.payload;
                call_path_string_length = getStringLength(call_client_pid); 
                call_path_string = (char*)malloc(call_path_string_length);
                getString(call_client_pid, call_path_string, call_path_string_length);
                vfs_getNthFileInfo(call_path_string, call_index, &file_info);
                postMessage(call_client_pid, FS_GET_NTH_FILE_TYPE, file_info.filetype);
                sendString(file_info.filename, call_client_pid);
                free(file_info.filename);
                free(call_path_string);
            break;

            default:
            break;
        }
    }
}
