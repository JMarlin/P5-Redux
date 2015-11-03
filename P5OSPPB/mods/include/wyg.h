#ifndef WYG_H
#define WYG_H

#define WYG_CREATE_WINDOW 1
#define WYG_GET_CONTEXT 2
#define WYG_MOVE_WINDOW 3
#define WYG_INSTALL_WINDOW 4
#define WYG_SHOW_WINDOW 5
#define WYG_REPAINT_WINDOW 6
#define WYG_POINT 7
#define WYG_WHANDLE 8
#define WYG_GET_DIMS 9
#define WYG_RAISE_WINDOW 10

#define ROOT_WINDOW 1

#define WIN_UNDECORATED 1
#define WIN_FIXEDSIZE 2
#define WIN_VISIBLE 4

struct bitmap;

unsigned int initWYG(void);
unsigned int createWindow(unsigned short width, unsigned short height, unsigned char flags);
struct bitmap* getWindowContext(unsigned int handle);
void moveWindow(unsigned int handle, unsigned short x, unsigned short y);
void installWindow(unsigned int child_handle, unsigned int parent_handle);
void showWindow(unsigned int handle);
void repaintWindow(unsigned int handle);
void getWindowDimensions(unsigned int handle, unsigned short *w, unsigned short *h); 
void focus(unsigned int handle);

#endif //WYG_H
