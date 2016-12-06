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
#define WYG_RESIZE_WINDOW  (WYG_MSG_CLASS | 15)
#define WYG_DRAW_STRING    (WYG_MSG_CLASS | 16)
#define WYG_EVENT          (WYG_MSG_CLASS | 17)

//Wyg event codes to be passed to the client
#define WYG_EVENT_REPAINT  0

#define ROOT_WINDOW 1

//Widget type declarations
#define WIDGET_TYPE_WINDOW  0x00000000 
#define WIDGET_TYPE_BUTTON  0x01000000
#define WIDGET_TYPE_TEXTBOX 0x02000000

//Some flags to define our window behavior
#define WIN_NODECORATION 0x1
#define WIN_NORAISE 0x2
#define WIN_BODYDRAG 0x4
#define WIN_HIDDEN 0x8

unsigned int initWYG(void);
unsigned int createWindow(unsigned int flags);
unsigned int getWindowContext(unsigned int handle);
void moveHandle(unsigned int handle, unsigned short x, unsigned short y);
void installWindow(unsigned int child_handle, unsigned int parent_handle);
void moveWindow(unsigned int handle, unsigned short x, unsigned short y);
void showWindow(unsigned int handle);
void hideWindow(unsigned int handle);
void repaintWindow(unsigned int handle);
void getWindowDimensions(unsigned int handle, unsigned short *w, unsigned short *h); 
void focus(unsigned int handle);
void setTitle(unsigned int handle, unsigned char* string);
void getWindowLocation(unsigned int handle, unsigned short* x, unsigned short* y);
void destroyWindow(unsigned int handle);
void getFrameDims(unsigned char* top, unsigned char* left, unsigned char* bottom, unsigned char* right);
void resizeWindow(unsigned int handle, unsigned short w, unsigned short h);
void updateMouse(short off_x, short off_y);
void drawString(unsigned int handle, unsigned short x, unsigned short y, char* c);

#endif //WYG_H
