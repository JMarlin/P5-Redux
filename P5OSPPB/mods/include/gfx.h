#ifndef GFX_H
#define GFX_H

#define GFX_MSG_CLASS ((unsigned int)0x00100000)

#define GFX_ENUMMODES   (GFX_MSG_CLASS | 1)
#define GFX_MODEDETAIL  (GFX_MSG_CLASS | 2)
#define GFX_SETMODE     (GFX_MSG_CLASS | 3)
#define GFX_SETCOLOR    (GFX_MSG_CLASS | 4)
#define GFX_SETCURSOR   (GFX_MSG_CLASS | 5)
#define GFX_SETPIXEL    (GFX_MSG_CLASS | 6)
#define GFX_DRAWHLINE   (GFX_MSG_CLASS | 7)
#define GFX_DRAWVLINE   (GFX_MSG_CLASS | 8)
#define GFX_DRAWRECT    (GFX_MSG_CLASS | 9)
#define GFX_FILLRECT   (GFX_MSG_CLASS | 10)
#define GFX_DRAWCHAR   (GFX_MSG_CLASS | 11)
#define GFX_DRAWSTRING (GFX_MSG_CLASS | 12)
#define GFX_DRAWBMP (GFX_MSG_CLASS | 13)
#define GFX_CPSCREEN (GFX_MSG_CLASS | 14)

#define RGB(r, g, b) (((r & 0xFF) << 16) + ((g & 0xFF) << 8) + (b & 0xFF))
#define RVAL(x) ((x & 0xFF0000) >> 16)
#define GVAL(x) ((x & 0xFF00) >> 8)
#define BVAL(x) (x & 0xFF)

typedef struct bitmap {
    
    //Dimensions of the image
    unsigned int height;
    unsigned int width;
    
    //Pointer to the raw image data
    unsigned int* data;

    //Window into the current portion
    //of the image to be drawn
    unsigned int top;
    unsigned int left;
    unsigned int bottom;
    unsigned int right;

    unsigned int mask_color; 
} bitmap;

typedef struct screen_mode {
    unsigned short width;
    unsigned short height;
    unsigned short depth;
    unsigned short number;
    unsigned char is_linear;
} screen_mode;

unsigned char initGfx();
void endGfx(); //Only for test harness use
unsigned char enumerateModes();
screen_mode* getModeDetails(unsigned short modenum);
unsigned char setScreenMode(unsigned short modenum);
void setColor(unsigned int color);
void setCursor(unsigned short x, unsigned short y);
void setPixel();
void drawHLine(unsigned short length);
void drawVLine(unsigned short length);
void drawRect(unsigned short width, unsigned short height);
void fillRect(unsigned short width, unsigned short height);
void drawChar(char c);
void drawStr(char* str);
bitmap* newBitmap(unsigned int width, unsigned int height);
void freeBitmap(bitmap* bmp);
void drawBitmap(bitmap* bmp);
void copyScreen(bitmap* bmp);

#endif //GFX_H
