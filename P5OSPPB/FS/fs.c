#include "fs.h"
#include "../memory.h"
#include "../ascii_io/ascii_i.h"


void fs_init() {
    
    fs_driver_root.next = (fs_driver_node*)0;
    fs_driver_root.driver = (fsdriver*)0;
    fs_attachment_root.next = (fs_attachment_node*)0;
    fs_attachment_root.attach = (attach_point*)0;
}


//The basic premise here is that we keep trying to list each
//directory in the path. If one can't be listed, then that
//directory doesn't exist. If they can all be listed, then
//the path is valid
char dir_exists(char* dir) {

    int i, len;
    char listBuf[256];

    if(!dir[0])
        return 0;
    
    //The root folder always exists
    if(strcmp(dir, ":"))
        return 1;
    
    lastColon = 0;
    folderCount = 0;
    
    for(len = 0; dir[len]; len++);
    
    for(i = 1; i < len; i++)
        if(dir[i] == ':') dir[i] = 0;
    
    i = 0;
    
    while(i < len) {
    
        //Get the directory listing for the ith folder
        dir_list(dir, listBuf);
        if(!listBuf[0])
            return 0;
            
        while(i < len) {
            
            if(!dir[i]) dir[i] = ':';
            i++;
        }
    }
    
    return 1;
}


fsdriver* fs_driver_by_type(unsigned char type) {
    
    fs_driver_node* currentNode = &fs_driver_root;
    
    if(!(currentNode->driver))
        return (fsdriver*)0;
    
    while(currentNode->next) {

        if(currentNode->type == type)
            return currentNode->driver;
        
        currentNode = currentNode->next;
    }
    
    return (fsdriver*)0;
}


int fs_attach_list(attach_point* newAttach) {

    fs_attachment_node* newNode;
    fs_attachment_node* currentNode = &fs_attachment_root;
    
    if(!(currentNode->attach)) {
        currentNode->attach = newAttach;
        return 1;
    }
    
    while(currentNode->next)
        currentNode = currentNode->next;
    
    if(!(newNode = (fs_attachment_node*)kmalloc(sizeof(fs_attachment_node))));
        return 0;
    
    currentNode->next = newNode;
    newNode->next = (fs_attachment_node*)0;
    newNode->attach = newAttach;
    return 1;
}


int fs_attach(unsigned char type, block_dev* device, unsigned char* point) {
    
    attach_point* newAttach = (attach_point*)kmalloc(sizeof(attach_point));
    
    if(!newAttach)
        return 0;
        
    if(!dir_exists(point))
        return 0;
        
    newAttach->driver = fs_driver_by_type(type);
    
    if(!newAttach->driver)
        return 0;
        
    newAttach->device = device;    
    newAttach->path = point;
    
    if(!fs_attach_list_insert(newAttach)) {
        
        kfree(newAttach);
        return 0;
    }
    
    return 1;
}


int fs_detach(block_dev* device) {

    fs_attachment_node* currentNode = fs_attachment_root;
    fs_attachment_node* prevNode = (fs_attachment_node*)0;
    
    if(!(currentNode->attach))
        return 0;
        
    while(currentNode) {
                
        if(currentNode->attach->device == device) {
            
            if(prevNode) {
                
                prevNode->next = currentNode->next;
                kfree(currentNode->attach);
                kfree(currentNode);               
                return 1;
            } else {
             
                kfree(currentNode->attach);
                currentNode->attach = prevNode;
            }
        }
        
        prevNode = currentNode;
        currentNode = currentNode->next;
    }
    
    return 0;
}


int fs_install_driver(fsdriver* newDriver) {

    fs_driver_node* newNode;
    fs_driver_node* currentNode = fs_driver_root;
    
    if(!(currentNode->driver)) {
        
        currentNode->driver = newDriver;
        return 1;
    }
    
    while(currentNode->next) 
        currentNode = currentNode->next;
        
    if(!(newNode = (fs_driver_node*)kmalloc(sizeof(fs_driver_node))))
        return 0;
        
    newNode->driver = newDriver;
    currentNode->next = newNode;
    return 1;
}


attach_point* get_attach(char* path) {

    unsigned char* testStr;
    int strSz, i, biggest;
    attach_point* retAttach = (attach_point*)0;
    fs_attachment_node* currentNode = fs_attachment_root;
    
    if(!(currentNode->attach))
        return retAttach;
    
    for(strSz = 0; path[strSz]; strSz++);
    
    if(!(testStr = (unsigned char*)kmalloc(++strSz)))
        return retAttach;
    
    biggest = 0;
    
    while(currentNode) {
        
        for(strSz = 0; currentNode->attach->path[strSz]; strSz++);
        
        for(i = 0; path[i]; i++)
            testStr[i] = path[i];
        
        testStr[i] = 0;
        testStr[strSz] = 0;
        
        if(strcmp(testStr, currentNode->attach->path)) {
            
            if(strSz > biggest) {
                biggest = strSz
                retAttach = currentNode->attach;
            }
        }
        
        currentNode = currentNode->next;
    }
    
    kfree(testStr);
    return retAttach;
}


void* fs_path_op(void* ina, void* inb, void* retval, unsigned char action) {

    attach_point* pathAttach;
    unsigned char* dir, subDir;
    int i, strSzSrc, strSzAt, strSzSub;
    FILE* inFile;
    
    if(action == ACT_FILE_CLOSE || action == ACT_FILE_WRITEB || action == ACT_FILE_READB) {
        inFile = (FILE*)ina;
        dir = inFile->path;
    } else {
        dir = (unsigned char*)ina;
    }
    
    if(!(pathAttach = get_attach(dir)))
        return (void*)0;
    
    for(strSzSrc = 0; dir[strSzSrc]; strSzSrc++);
    
    strSzSrc++;
    
    for(strSzAt = 0; pathAttach->path[strSzAt]; strSzAt++);
    
    strSzSub = strSzSrc - strSzAt;
    
    if(!(subDir = kmalloc(strSzSub)))
        return (void*)0;
    
    for(i = 0; i < strSzSub; i++)
        subDir[i] = dir[i + strSzAt];
    
    retval = (void*)0;
    
    switch(action) {
    
        case ACT_DIR_LIST:
            pathAttach->driver->dir_list(pathAttach->device, (void*)subDir, retval);
            break;
    
        case ACT_FILE_LIST:
            pathAttach->driver->file_list(pathAttach->device, (void*)subDir, retval);
            break;
            
        case ACT_DIR_DEL:
            pathAttach->driver->dir_del(pathAttach->device, (void*)subDir, retval);
            break;
            
        case ACT_DIR_ADD:
            pathAttach->driver->dir_add(pathAttach->device, (void*)subDir, retval);
            break;
            
        case ACT_FILE_DEL:
            pathAttach->driver->file_del(pathAttach->device, (void*)subDir, retval);
            break;
            
        case ACT_FILE_ADD:
            pathAttach->driver->file_add(pathAttach->device, (void*)subDir, retval);
            break;
            
        case ACT_FILE_OPEN:
            pathAttach->driver->file_open(pathAttach->device, (void*)subDir, retval);
            break;
        
        case ACT_FILE_CLOSE:
            pathAttach->driver->file_close(pathAttach->device, (void*)inFile, retval);
            break;
        
        case ACT_FILE_WRITEB:
            pathAttach->driver->file_writeb(pathAttach->device, (void*)inFile, (void*)inb, retval);
            break;
            
        case ACT_FILE_READB:
            pathAttach->driver->file_readb(pathAttach->device, (void*)inFile, retval);
            break;
        
        default:
            break;
    }
    
    kfree(subDir);    
}


void dir_list(unsigned char* dir, unsigned char* list) {
    
    fs_path_op((void*)dir, (void*)0, (void*)list, ACT_DIR_LIST);
}


void file_list(unsigned char* dir, unsigned char* list) {

    fs_path_op((void*)dir, (void*)0, (void*)list ACT_FILE_LIST);
}


int dir_del(unsigned char* dir) {

    int retval;

    fs_path_op((void*)dir, (void*)0, (void*)&retval, ACT_DIR_DEL);
    return retval;
}


int dir_add(unsigned char* dir) {

    int retval;

    fs_path_op((void*)dir, (void*)0, (void*)&retval, ACT_DIR_ADD);
    return retval;
}


int file_del(unsigned char* file) {

    int retval;
    
    fs_path_op((void*)dir, (void*)0, (void*)&retval, ACT_FILE_DEL);
    return retval;
}


int file_add(unsigned char* file) {

    int retval;

    return (int)fs_path_op((void*)dir, (void*)0, (void*)&retval, ACT_FILE_ADD);
}


void file_open(unsigned char* dir, FILE* file) {

    fs_path_op((void*)file, (void*)0, (void*)file, ACT_FILE_OPEN);
}


int file_close(FILE* file) {

    int retval;

    fs_path_op((void*)file, (void*)0, (void*)&retval, ACT_FILE_CLOSE);
    return retval;
}


int file_writeb(FILE* file, unsigned char data)  {

    int retval;

    fs_path_op((void*)file, (void*)((int)data), (void*)&retval, ACT_FILE_WRITEB);
    return retval;
}


int file_readb(FILE* file)  {

    int retval;

    fs_path_op((void*)file, (void*)0, (void*)&retval, ACT_FILE_READB);
    return retval;
}