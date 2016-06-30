#ifndef WYG_H
#define WYG_H

#define WYG_MSG_CLASS ((unsigned int)0x00600000)

#define WYG_CREATE_WINDOW  (WYG_MSG_CLASS | 1)
#define WYG_GET_CONTEXT    (WYG_MSG_CLASS | 2)
#define WYG_MOVE_WINDOW    (WYG_MSG_CLASS | 3)
#define WYG_INSTALL_WINDOW (WYG_MSG_CLASS | 4)
#define WYG_SHOW_WINDOW    (WYG_MSG_CLASS | 5)
#define WYG_REPAINT_WINDOW (WYG_MSG_CLASS | 6)
#define WYG_POINT          (WYG_MSG_CLASS | 7)
#define WYG_WHANDLE        (WYG_MSG_CLASS | 8)
#define WYG_GET_DIMS       (WYG_MSG_CLASS | 9)
#define WYG_RAISE_WINDOW   (WYG_MSG_CLASS | 10)
#define WYG_SET_TITLE      (WYG_MSG_CLASS | 11)
#define WYG_GET_LOCATION   (WYG_MSG_CLASS | 12)
#define WYG_DESTROY        (WYG_MSG_CLASS | 13)
#define WYG_GET_FRAME_DIMS (WYG_MSG_CLASS | 14)

#define ROOT_WINDOW 1

#define WIN_UNDECORATED 1
#define WIN_FIXEDSIZE 2
#define WIN_VISIBLE 4

struct bitmap;

unsigned int initWYG(void);
unsigned int createWindow(unsigned short width, unsigned short height, unsigned char flags);
struct bitmap* getWindowContext(unsigned int handle);
void moveHandle(unsigned int handle, unsigned short x, unsigned short y);
void installWindow(unsigned int child_handle, unsigned int parent_handle);
void showWindow(unsigned int handle);
void repaintWindow(unsigned int handle);
void getWindowDimensions(unsigned int handle, unsigned short *w, unsigned short *h); 
void focus(unsigned int handle);
void setTitle(unsigned int handle, unsigned char* string);
void getWindowLocation(unsigned int handle, unsigned short* x, unsigned short* y);
void destroyWindow(unsigned int handle);
void getFrameDims(unsigned char* top, unsigned char* left, unsigned char* bottom, unsigned char* right);
void updateMouse(short off_x, short off_y);

#endif //WYG_H
