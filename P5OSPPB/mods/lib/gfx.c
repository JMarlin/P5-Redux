#include "../include/gfx.h"
#include "../include/p5.h"
#include "../include/registrar.h"

unsigned int gfx_pid;
message temp_msg;
screen_mode mode_details;

unsigned char initGfx() {

    //Find the GFX server
    //Try a few times in case the service is in the middle of registering
    int i;
    for(i = 0; i < 100; i++) {
    
    	postMessage(REGISTRAR_PID, REG_LOOKUP, SVC_GFX);
    	getMessage(&temp_msg);
    	
	if(temp_msg.payload)
	    break;
    }
    gfx_pid = temp_msg.payload;

    return gfx_pid != 0;
}

//Simply returns the number of modes supported
unsigned char enumerateModes() {

    prints("\nSending enumeration request to gfx server\n");
    postMessage(gfx_pid, GFX_ENUMMODES, 0);
    prints("Listening for response\n");
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
    mode_details.width = (unsigned short)(((temp_msg.payload & 0xFFFE0000) >> 17) & 0xFFFF);
    mode_details.height = (unsigned short)(((temp_msg.payload & 0x0001FFFC) >> 2) & 0xFFFF);

    //The bit of math in here is representative of the fact that we support 8-bit
    //16-bit, 24-bit or 32-bit color
    mode_details.depth = (unsigned char)(((temp_msg.payload & 0x3) + 1) * 8);
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
