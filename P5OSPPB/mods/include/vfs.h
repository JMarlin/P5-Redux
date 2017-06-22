#ifndef VFS_H
#define VFS_H

#define VFS_MSG_CLASS ((unsigned int)0x00900000)

//MOVE TO A VFS INTERFACE LIBRARY
#define VFS_REGISTER_BLOCK (VFS_MSG_CLASS | 0x1)
#define VFS_REGISTER_FS    (VFS_MSG_CLASS | 0x2)

#endif //VFS_H