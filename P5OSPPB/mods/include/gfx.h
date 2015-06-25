#ifndef GFX_H
#define GFX_H


#define GFX_ENUMMODES   1
#define GFX_MODEDETAIL  2
#define GFX_SETMODE     3
#define GFX_SETCOLOR    4
#define GFX_SETCURSOR   5
#define GFX_SETPIXEL    6
#define GFX_DRAWHLINE   7
#define GFX_DRAWVLINE   8
#define GFX_DRAWRECT    9
#define GFX_FILLRECT   10
#define GFX_DRAWCHAR   11
#define GFX_DRAWSTRING 12

#define SVC_GFX 1

#define RGB(r, g, b) (((r & 0xFF) << 16) + ((g & 0xFF) << 8) + (b & 0xFF))
#define RVAL(x) ((x & 0xFF0000) >> 16)
#define GVAL(x) ((x & 0xFF00) >> 8)
#define BVAL(x) (x & 0xFF)

typedef struct screen_mode {
    unsigned short width;
    unsigned short height;
    unsigned short depth;
    unsigned short number;
} screen_mode;

unsigned char initGfx();
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

#endif //GFX_H