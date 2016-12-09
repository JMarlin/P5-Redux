#include "../include/wyg.h"
#include "../include/p5.h"
#include "../include/registrar.h"
#include "../include/gfx.h"

message temp_msg;
unsigned int wyg_pid;

unsigned int initWYG(void) {
	
	//Find the WYG server
	postMessage(REGISTRAR_PID, REG_LOOKUP, SVC_WYG);
    getMessage(&temp_msg);
    wyg_pid = temp_msg.payload;

    return wyg_pid != 0;
}

unsigned int createWindow(unsigned int flags) {
	
	postMessage(wyg_pid, WYG_CREATE_WINDOW, flags);
	getMessageFrom(&temp_msg, wyg_pid, WYG_CREATE_WINDOW);
	
	return temp_msg.payload;
}

unsigned int getWindowContext(unsigned int handle) {
	
	postMessage(wyg_pid, WYG_GET_CONTEXT, handle);
	getMessageFrom(&temp_msg, wyg_pid, WYG_GET_CONTEXT);
	
	return temp_msg.payload;
}

void moveWindow(unsigned int handle, unsigned short x, unsigned short y) {
	
	postMessage(wyg_pid, WYG_MOVE_WINDOW, handle);
	postMessage(wyg_pid, WYG_POINT, (((unsigned int)x & 0xFFFF) << 16) | ((unsigned int)y & 0xFFFF));
}

void installWindow(unsigned int child_handle, unsigned int parent_handle) {
	
	postMessage(wyg_pid, WYG_INSTALL_WINDOW, child_handle);
	postMessage(wyg_pid, WYG_WHANDLE, parent_handle);
}

void showWindow(unsigned int handle) {
	
	postMessage(wyg_pid, WYG_SHOW_WINDOW, handle);
}

void repaintWindow(unsigned int handle) {
	
	postMessage(wyg_pid, WYG_REPAINT_WINDOW, handle);
	getMessageFrom(&temp_msg, wyg_pid, WYG_REPAINT_WINDOW);
}

void getWindowDimensions(unsigned int handle, unsigned short *w, unsigned short *h) {
	
	postMessage(wyg_pid, WYG_GET_DIMS, handle);
	getMessageFrom(&temp_msg, wyg_pid, WYG_GET_DIMS);
	
	*w = (unsigned short)((temp_msg.payload & 0xFFFF0000) >> 16);
	*h = (unsigned short)(temp_msg.payload & 0xFFFF);
}

void focus(unsigned int handle) {
	
	postMessage(wyg_pid, WYG_RAISE_WINDOW, handle);
}

void setTitle(unsigned int handle, unsigned char* string) {
	
	postMessage(wyg_pid, WYG_SET_TITLE, handle);
	getMessageFrom(&temp_msg, wyg_pid, WYG_SET_TITLE);
	sendString(string, wyg_pid);
}

void getWindowLocation(unsigned int handle, unsigned short* x, unsigned short* y) {
	
	postMessage(wyg_pid, WYG_GET_LOCATION, handle);
	getMessageFrom(&temp_msg, wyg_pid, WYG_GET_LOCATION);
	
	*x = (unsigned short)((temp_msg.payload & 0xFFFF0000) >> 16);
	*y = (unsigned short)(temp_msg.payload & 0xFFFF);
}

void destroyWindow(unsigned int handle) {
	
	postMessage(wyg_pid, WYG_DESTROY, handle);
	getMessageFrom(&temp_msg, wyg_pid, WYG_DESTROY);
}

void getFrameDims(unsigned char* top, unsigned char* left, unsigned char* bottom, unsigned char* right) {
	
	postMessage(wyg_pid, WYG_GET_FRAME_DIMS, 1);
	getMessageFrom(&temp_msg, wyg_pid, WYG_GET_FRAME_DIMS);
	
	(*top) = (unsigned char)((temp_msg.payload >> 24) & 0xFF);
	(*left) = (unsigned char)((temp_msg.payload >> 16) & 0xFF);
	(*bottom) = (unsigned char)((temp_msg.payload >> 8) & 0xFF);
	(*right) = (unsigned char)(temp_msg.payload & 0xFF);
}

void resizeWindow(unsigned int handle, unsigned short w, unsigned short h) {
	
	postMessage(wyg_pid, WYG_RESIZE_WINDOW, handle);
	postMessage(wyg_pid, WYG_POINT, (((unsigned int)w & 0xFFFF) << 16) | ((unsigned int)h & 0xFFFF));
}

void drawString(unsigned int handle, unsigned short x, unsigned short y, char* c) {

	postMessage(wyg_pid, WYG_DRAW_STRING, handle);
	postMessage(wyg_pid, WYG_POINT, (((unsigned int)x & 0xFFFF) << 16) | ((unsigned int)y & 0xFFFF));
    getMessageFrom(&temp_msg, wyg_pid, WYG_DRAW_STRING);
	sendString(c, wyg_pid);
}

void drawRectangle(unsigned int handle, unsigned short x, unsigned short y,
                   unsigned short w, unsigned short h, unsigned int color) {

    postMessage(wyg_pid, WYG_DRAW_RECT, handle);
	postMessage(wyg_pid, WYG_POINT, (((unsigned int)x & 0xFFFF) << 16) | ((unsigned int)y & 0xFFFF));
    postMessage(wyg_pid, WYG_DIMS, (((unsigned int)w & 0xFFFF) << 16) | ((unsigned int)h & 0xFFFF))
	postMessage(wyg_pid, WYG_COLOR, color);
	getMessageFrom(&temp_msg, wyg_pid, WYG_DRAW_RECT);
}