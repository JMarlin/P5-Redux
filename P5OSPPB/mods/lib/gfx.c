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

void* getFramebuffer() {

    postMessage(gfx_pid, GFX_GETFB, 0);
    getMessageFrom(&temp_msg, gfx_pid, GFX_GETFB);

    return (void*)temp_msg.payload;
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
    getMessageFrom(&temp_msg, gfx_pid, GFX_SETCURSOR);
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

bitmap* newBitmap(unsigned int width, unsigned int height) {

    unsigned int bmp_size = width * height;
    unsigned int bufsz = (bmp_size *  sizeof(unsigned int)) + sizeof(bitmap);
    bitmap* return_bmp;
    unsigned int i;

    //Ceil bufsz to the next page
    bufsz = ((bufsz / 0x1000) * 0x1000) + ((bufsz % 0x1000) ? 0x1000 : 0);
    
    //Allocate a shared memory region (needs to be shared so that the GFX server can access it)
    //cmd_prints("Allocating ");
    //cmd_printDecimal(bufsz >> 12);
    //cmd_prints(" pages of shared memory...");
    
    if(!(return_bmp = (bitmap*)getSharedPages((bufsz >> 12) + 1)))
        return (bitmap*)0;
    
    //cmd_prints("Done (");
    //cmd_printHexDword(return_bmp);
    //cmd_prints(")\nSetting bitmap dimensions...");
    
    //Set dimensions    
    return_bmp->height = height;
    return_bmp->width = width;
    
    //Default the window to max
    return_bmp->top = 0;
    return_bmp->left = 0;
    return_bmp->bottom = return_bmp->height - 1;
    return_bmp->right = return_bmp->width - 1;
    
    //Plug in the data region
    //cmd_prints("Done\nInitializing data buffer...");
    return_bmp->data = (unsigned int*)((unsigned char*)return_bmp + sizeof(bitmap));
    
    //Clear the bitmap
    /*TEMP IGNORE
    for(i = 0; i < bmp_size; i++) {

        ((unsigned int*)0xB00000)[3] = i;
        
        if(i == bmp_size)
            break;
        
        return_bmp->data[i] = 0;
    }
    */
    return_bmp->mask_color = 0;
    
    //cmd_prints("Done\n");
        
    return return_bmp;
}

void freeBitmap(bitmap* bmp) {
    
    freeSharedPages((void*)bmp);
}

void drawBitmap(bitmap* bmp) {
    
    postMessage(gfx_pid, GFX_DRAWBMP, (unsigned int)bmp);
    getMessageFrom(&temp_msg, gfx_pid, GFX_DRAWBMP);
}

void copyScreen(bitmap* bmp) {
    
    postMessage(gfx_pid, GFX_CPSCREEN, (unsigned int)bmp);
    getMessageFrom(&temp_msg, gfx_pid, GFX_CPSCREEN);
}
