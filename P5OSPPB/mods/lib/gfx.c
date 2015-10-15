#include "../include/gfx.h"
#include "../include/p5.h"
#include "../include/registrar.h"

unsigned int gfx_pid;
message temp_msg;
screen_mode mode_details;

unsigned char initGfx() {

    //Find the GFX server
	postMessage(REGISTRAR_PID, REG_LOOKUP, SVC_GFX);
    getMessage(&temp_msg);
    gfx_pid = temp_msg.payload;

    return gfx_pid != 0;
}

//Simply returns the number of modes supported
unsigned char enumerateModes() {

    postMessage(gfx_pid, GFX_ENUMMODES, 0);
    getMessage(&temp_msg);

    return (unsigned char)(temp_msg.payload & 0xFF);
}

//Important note: The modenum here is not a VESA or VGA mode number,
//it's simply the index into the number of modes the GFX server has
//enumerated
//As well, mode 0 is reserved for text mode
screen_mode* getModeDetails(unsigned short modenum) {

    if(!modenum)
        return (screen_mode*)0;

    postMessage(gfx_pid, GFX_MODEDETAIL, (unsigned int)modenum);
    getMessage(&temp_msg);

    //Unpack the passed values
    mode_details.width = (unsigned short)(((temp_msg.payload & 0xFFF80000) >> 19) & 0x1FFF);
    mode_details.height = (unsigned short)(((temp_msg.payload & 0x0007FFC0) >> 6) & 0x1FFF);

    //The bit of math in here is representative of the fact that we support 8-bit
    //16-bit, 24-bit or 32-bit color
    mode_details.depth = (unsigned short)(((temp_msg.payload & 0x3) + 1) * 8);
	mode_details.is_linear = (unsigned char)((temp_msg.payload & 0x4) != 0);
    mode_details.number = 0; //Only the server cares about this

    return &mode_details;
}

unsigned char setScreenMode(unsigned short modenum) {

    //Set the mode and detect whether it took
    postMessage(gfx_pid, GFX_SETMODE, (unsigned int)modenum);
    getMessage(&temp_msg);

    return (unsigned char)(temp_msg.payload & 0xFF);
}

void setColor(unsigned int color) {

    postMessage(gfx_pid, GFX_SETCOLOR, color);
}

void setCursor(unsigned short x, unsigned short y) {

    postMessage(gfx_pid, GFX_SETCURSOR, ((unsigned int)x << 16) | (unsigned int)y);
}

void setPixel() {

    postMessage(gfx_pid, GFX_SETPIXEL, 0);
}

void drawHLine(unsigned short length) {

    postMessage(gfx_pid, GFX_DRAWHLINE, (unsigned int)length);
}

void drawVLine(unsigned short length) {

    postMessage(gfx_pid, GFX_DRAWVLINE, (unsigned int)length);
}

void drawRect(unsigned short width, unsigned short height) {

    postMessage(gfx_pid, GFX_DRAWRECT, ((unsigned int)width << 16) | (unsigned int)height);
}

void fillRect(unsigned short width, unsigned short height) {

    postMessage(gfx_pid, GFX_FILLRECT, ((unsigned int)width << 16) | (unsigned int)height);
}

void drawChar(char c) {

    postMessage(gfx_pid, GFX_DRAWCHAR, (unsigned int)c);
}

void drawStr(char* str) {

    postMessage(gfx_pid, GFX_DRAWSTRING, (unsigned int)str);
}
