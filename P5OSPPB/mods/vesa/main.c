#include "../include/p5.h"
#include "../include/registrar.h"
#include "../include/gfx.h"
#include "../include/memory.h"
#include "font.h" //Should eventually make this into a binary font file loadable via the filesystem

void getModes(void);
int setMode(unsigned short mode);
void plotPixel(int x, int y, unsigned int color);
void VdrawHLine(int x, int y, int length, unsigned int color);
void VdrawVLine(int x, int y, int length, unsigned int color);
void VdrawRect(int x, int y, int width, int height, unsigned int color);
void VfillRect(int x, int y, int width, int height, unsigned int color);
void drawCharacter(unsigned char c, int x, int y, unsigned int color);
void VdrawBitmap(int x, int y, bitmap* bmp);
void VcopyScreen(int x, int y, bitmap* bmp);

message temp_msg;
unsigned int pen_color = RGB(255,0,0);
unsigned short pen_x = 0;
unsigned short pen_y = 0;
unsigned char mode_count = 0;
unsigned char is_linear = 0;
unsigned int* v = (unsigned int*)0;

screen_mode detected_modes[10];

typedef struct ModeInfoBlock {
  unsigned short attributes;
  unsigned char winA,winB;
  unsigned short granularity;
  unsigned short winsize;
  unsigned short segmentA, segmentB;
  unsigned short realFctPtrSeg;
  unsigned short realFctPtrOff;
  unsigned short pitch;

  unsigned short Xres, Yres;
  unsigned char Wchar, Ychar, planes, bpp, banks;
  unsigned char memory_model, bank_size, image_pages;
  unsigned char reserved0;

  unsigned char red_mask, red_position;
  unsigned char green_mask, green_position;
  unsigned char blue_mask, blue_position;
  unsigned char rsv_mask, rsv_position;
  unsigned char directcolor_attributes;

  unsigned int physbase;
  unsigned int reserved1;
  unsigned short reserved2;
} ModeInfoBlock;

typedef struct VESAInfo {
    unsigned char sig[4];
    unsigned short version;
    unsigned short OEMStringPtrOff;
    unsigned short OEMStringPtrSeg;
    unsigned char capabilities[4];
    unsigned short modePtrOff;
    unsigned short modePtrSeg;
    unsigned short memSize;
    unsigned char reserved;
} VESAInfo;

ModeInfoBlock curMode;
unsigned int client_pid = 0;
unsigned char curBank = 0;

/*
void dumpMode(ModeInfoBlock* mode) {

    unsigned char fake[10];

    pchar('\n');

    printHexWord(mode->attributes); pchar('|');
    printHexByte(mode->winA); pchar('|');
    printHexByte(mode->winB); pchar('|');
    printHexWord(mode->granularity); pchar('|');
    printHexWord(mode->winsize); pchar('|');
    printHexWord(mode->segmentA); pchar('|');
    printHexWord(mode->segmentB); pchar('|');
    printHexWord(mode->realFctPtrSeg); pchar('|');
    printHexWord(mode->realFctPtrOff); pchar('|');
    printHexWord(mode->pitch); pchar('|');

    printHexWord(mode->Xres); pchar('|');
    printHexWord(mode->Yres); pchar('|');
    printHexByte(mode->Wchar); pchar('|');
    printHexByte(mode->Ychar); pchar('|');
    printHexByte(mode->planes); pchar('|');
    printHexByte(mode->bpp); pchar('|');
    printHexByte(mode->banks); pchar('|');
    printHexByte(mode->memory_model); pchar('|');
    printHexByte(mode->bank_size); pchar('|');
    printHexByte(mode->image_pages); pchar('|');
    printHexByte(mode->reserved0); pchar('|');

    printHexByte(mode->red_mask); pchar('|');
    printHexByte(mode->red_position); pchar('|');
    printHexByte(mode->green_mask); pchar('|');
    printHexByte(mode->green_position); pchar('|');
    printHexByte(mode->blue_mask); pchar('|');
    printHexByte(mode->blue_position); pchar('|');
    printHexByte(mode->rsv_mask); pchar('|');
    printHexByte(mode->rsv_position); pchar('|');
    printHexByte(mode->directcolor_attributes); pchar('|');

    printHexDword(mode->physbase); pchar('|');
    printHexDword(mode->reserved1); pchar('|');
    printHexWord(mode->reserved2);

    pchar('\n');

    scans(5, fake);
}
*/

void main(void) {

    unsigned short new_mode;
    unsigned int parent_pid;
    unsigned int oldsrc;

    //Get the 'here's my pid' message from init
    getMessage(&temp_msg);
    parent_pid = temp_msg.source;
    prints("[VESA] Starting GFX services.\n");

    //First thing, register as a GFX service with the registrar
    postMessage(REGISTRAR_PID, REG_REGISTER, SVC_GFX);
    getMessage(&temp_msg);

    prints("[VESA] starting v86 helper...");
    //Then start the v86 helper thread
    client_pid = startV86(":v86.mod");
    prints("started.\n");

    if(!temp_msg.payload || !client_pid) {

        prints("\n[VESA] failed to register or start v86 thread.\n");
        postMessage(REGISTRAR_PID, REG_DEREGISTER, SVC_GFX);
        postMessage(parent_pid, 0, 0); //Tell the parent we're done registering
        terminate();
    }

    postMessage(parent_pid, 0, 1); //Tell the parent we're done registering

    //Now we can start the main message loop and begin handling
    //GFX command messages
    while(1) {

        //This here is a really good example of why we need
        //an 'ignore message' call -- shit could crash down
        //fast if we get a GFX request while we're waiting
        //on a response from v86
        getMessage(&temp_msg);
        oldsrc = temp_msg.source;

        switch(temp_msg.command) {

            case GFX_ENUMMODES:
                getModes();
                postMessage(temp_msg.source, GFX_ENUMMODES, (unsigned int)mode_count);
            break;

            case GFX_MODEDETAIL:
                if(temp_msg.payload > mode_count || !temp_msg.payload) {

                    postMessage(temp_msg.source, GFX_MODEDETAIL, 0);
                } else {

                    postMessage(temp_msg.source, GFX_MODEDETAIL,
                        ((unsigned int)(detected_modes[temp_msg.payload - 1].width & 0x1FFF) << 19) |
                        ((unsigned int)(detected_modes[temp_msg.payload - 1].height & 0x1FFF) << 6) |
                        ((unsigned int)(detected_modes[temp_msg.payload - 1].is_linear ? 4 : 0)) |
                        ((unsigned int)((detected_modes[temp_msg.payload - 1].depth / 8) - 1) & 3)
                    );
                }
            break;

            case GFX_SETMODE:
                if(!temp_msg.payload)
                    new_mode = 0x000C; //Mode 0 is always textmode
                else
                    new_mode = detected_modes[temp_msg.payload - 1].number;

                oldsrc = temp_msg.source;

                postMessage(oldsrc, GFX_SETMODE, setMode(detected_modes[temp_msg.payload - 1].is_linear ? (new_mode | 0x4000) : new_mode));
            break;

            case GFX_SETCOLOR:
                pen_color = temp_msg.payload;
            break;

            case GFX_SETCURSOR:
                pen_x = (unsigned short)((temp_msg.payload & 0xFFFF0000) >> 16);
                pen_y = (unsigned short)(temp_msg.payload & 0xFFFF);
                postMessage(oldsrc, GFX_SETCURSOR, 1);
            break;

            case GFX_SETPIXEL:
                plotPixel(pen_x, pen_y, pen_color);
            break;

            case GFX_DRAWHLINE:
                VdrawHLine(pen_x, pen_y, temp_msg.payload, pen_color);
            break;

            case GFX_DRAWVLINE:
                VdrawVLine(pen_x, pen_y, temp_msg.payload, pen_color);
            break;

            case GFX_DRAWRECT:
                VdrawRect(pen_x, pen_y, (unsigned short)((temp_msg.payload & 0xFFFF0000) >> 16), (unsigned short)(temp_msg.payload & 0xFFFF), pen_color);
            break;

            case GFX_FILLRECT:
                VfillRect(pen_x, pen_y, (unsigned short)((temp_msg.payload & 0xFFFF0000) >> 16), (unsigned short)(temp_msg.payload & 0xFFFF), pen_color);
            break;

            case GFX_DRAWCHAR:
                drawCharacter(temp_msg.payload & 0xFF, pen_x, pen_y, pen_color);
            break;

            case GFX_DRAWSTRING:
                //CAN'T DO THIS UNTIL WE SOLVE SHARED MEMORY
            break;

            case GFX_DRAWBMP:
                VdrawBitmap(pen_x, pen_y, (bitmap*)temp_msg.payload);
                postMessage(oldsrc, GFX_DRAWBMP, 1);
            break;
            
            case GFX_CPSCREEN:
                VcopyScreen(pen_x, pen_y, (bitmap*)temp_msg.payload);
                postMessage(oldsrc, GFX_CPSCREEN, 1);
            break;

            case GFX_GETFB:
                postMessage(oldsrc, GFX_GETFB, (unsigned int)v);
            break;

            default:
            break;
        }
    }
}

void setVESABank(unsigned char bank_no) {

    message tmp_msg;

    postMessage(client_pid, 4, bank_no);

    //HERE'S OUR PROBLEM!
    //This is picking up other drawing commands in the queue
    //instead of the response from the v86 module
    getMessageFrom(&tmp_msg, client_pid, 4);
}


ModeInfoBlock* getModeInfo(unsigned short mode) {

    message tmp_msg;
    unsigned short seg, off;

    postMessage(client_pid, 2, mode);
    getMessageFrom(&tmp_msg, client_pid, 2);
    seg = (unsigned short)(tmp_msg.payload & 0xFFFF);
    getMessageFrom(&tmp_msg, client_pid, 2);
    off = (unsigned short)(tmp_msg.payload & 0xFFFF);

    //return (ModeInfoBlock*)0x83000; //Should NOT do this
    //MAKE SURE THE BELOW VERSION WORKS
    return (ModeInfoBlock*)(((seg) << 4) + off);
}


void getModes(void) {

    message tmp_msg;
    unsigned short* modeList;
    ModeInfoBlock* info;
    VESAInfo* vinfo;
    unsigned short seg, off;
    int i;

    postMessage(client_pid, 1, 0);
    getMessageFrom(&tmp_msg, client_pid, 1);
    seg = (unsigned short)(tmp_msg.payload & 0xFFFF);
    postMessage(client_pid, 1, 0);
    getMessageFrom(&tmp_msg, client_pid, 1);

    off = (unsigned short)(tmp_msg.payload & 0xFFFF);
    vinfo = (VESAInfo*)((((unsigned int)seg) << 4) + off);

    modeList = (unsigned short*)((((unsigned int)vinfo->modePtrSeg) << 4) + vinfo->modePtrOff);

    i = 0;

    while(modeList[i] != 0xFFFF) {

        info = getModeInfo(modeList[i]);

        if((info->bpp >= 32) && (info->attributes & 0x1) && (mode_count < 10)) {

             //dumpMode(info);

             detected_modes[mode_count].number = modeList[i];
             detected_modes[mode_count].width = info->Xres;
             detected_modes[mode_count].height = info->Yres;
             detected_modes[mode_count].is_linear = info->physbase == 0 ? 0 : 1;
             detected_modes[mode_count].depth = info->bpp;
             mode_count++;
        }

        i++;
    }
}


int setMode(unsigned short mode) {

    message tmp_msg;
    unsigned char *tmp_info, *cast_mode;
    int i;

    postMessage(client_pid, 3, mode);

    //Should include timeouts for message waits like this
    getMessageFrom(&tmp_msg, client_pid, 3);

    //Make sure we didn't get an error
    if((tmp_msg.payload & 0xFF) == 0x4F) {

        if(tmp_msg.payload & 0x0100) {

            return 0;
        }
    } else {

        return 0;
    }

    //Get a permanent copy of the mode's info
    tmp_info = (unsigned char*)getModeInfo(mode);
    cast_mode = (unsigned char*)&curMode;

    for(i = 0; i < sizeof(ModeInfoBlock); i++)
        cast_mode[i] = tmp_info[i];

    //Here we should do some precalculation crap to speed
    //up pixel plotting and junk
    is_linear = curMode.physbase == 0 ? 0 : 1;

    if(is_linear) {

        v = (unsigned int*)allocatePhysical((void*)curMode.physbase, curMode.Yres * curMode.pitch);

        if(!v) {

            prints("\n[VESA] Could not allocate linear video memory.");
            return 0;
        }

        prints("Allocated memory, clearing framebuffer.\n");
        //Test to see if the mode switch completed
        for(i = 0; i < (curMode.Yres * curMode.pitch/4) ; i++)
            v[i] = 0xFF00FFFF;
             
    } else {

        //Should really get this off the window base from the mode info
        v = (unsigned int*)((unsigned int)curMode.segmentA << 4);

        //Test to see if the mode switch completed
        for(i = 0; i < 0x4000; i++)
            v[i] = 0xFFFF0000;
    }

    prints("Done setting mode\n");
    return 1;
}


void plotPixel32(int x, int y, int color) {

    unsigned int linear_pos = x + (y * (curMode.pitch/4));
    unsigned int window_pos;
    unsigned char bank_number;

    if(is_linear) {

        v[linear_pos] = (unsigned int)((RVAL(color) & 0xFF) << curMode.red_position) | ((GVAL(color) & 0xFF) << curMode.green_position) | ((BVAL(color) & 0xFF) << curMode.blue_position);
    } else {

        window_pos = linear_pos % (curMode.winsize * 0x100);
        bank_number = (unsigned char)((linear_pos / (curMode.winsize * 0x100)) & 0xFF);

        if(bank_number != curBank) {

            curBank = bank_number;
            setVESABank(curBank);
        }

        v[window_pos] = (unsigned int)((RVAL(color) & 0xFF) << curMode.red_position) | ((GVAL(color) & 0xFF) << curMode.green_position) | ((BVAL(color) & 0xFF) << curMode.blue_position);
    }
}


void plotPixel24(int x, int y, int color) {

    unsigned int linear_pos = y * curMode.pitch + (x * 3);
    unsigned int window_pos = linear_pos % ((curMode.winsize * 0x400) / 3);
    unsigned char bank_number = (unsigned char)((linear_pos / ((curMode.winsize * 0x400) / 3)) & 0xFF);
    unsigned int pixel = (unsigned int)((RVAL(color) & 0xFF) << curMode.red_position) | ((GVAL(color) & 0xFF) << curMode.green_position) | ((BVAL(color) & 0xFF) << curMode.blue_position);

    if(bank_number != curBank) {

        curBank = bank_number;
        setVESABank(curBank);
    }

    //We do it in this order because we're little-endian
    v[window_pos++] = pixel & 0xFF;
    v[window_pos++] = (pixel >> 8) & 0xFF;
    v[window_pos] = (pixel >> 16) & 0xFF;
}


void plotPixel16(int x, int y, int color) {

    unsigned int linear_pos = y * curMode.pitch + x;
    unsigned int window_pos = linear_pos % (curMode.winsize >> 1);
    unsigned char bank_number = (unsigned char)((linear_pos / (curMode.winsize >> 1)) & 0xFF);

    if(bank_number != curBank) {

        curBank = bank_number;
        setVESABank(curBank);
    }

    v[window_pos] = (unsigned short)((RVAL(color) & 0xF) << curMode.red_position) | ((GVAL(color) & 0xF) << curMode.green_position) | ((BVAL(color) & 0xF) << curMode.blue_position);
}


//Should replace this with a dynamically set function
//pointer when the video mode is set
void plotPixel(int x, int y, unsigned int color) {

    //Don't draw pixels outside of the framebuffer
    if(x < 0 || x >= curMode.Xres || y < 0 || y >= curMode.Yres)
        return;

    if(curMode.bpp == 32) plotPixel32(x, y, color);
    if(curMode.bpp == 24) plotPixel24(x, y, color);
    if(curMode.bpp == 16) plotPixel16(x, y, color);
}


void VdrawHLine(int x, int y, int length, unsigned int color) {

    int i, endx;

    endx = x + length;

    for(i = x; i < endx; i++)
        plotPixel(i, y, color);
}

void VdrawVLine(int x, int y, int length, unsigned int color) {

    int i, endy;

    endy = length + y;

    for(i = y; i < endy; i++)
        plotPixel(x, i, color);
}


void VdrawRect(int x, int y, int width, int height, unsigned int color) {

    VdrawHLine(x, y, width, color);
    VdrawVLine(x, y, height, color);
    VdrawHLine(x, y + height - 1, width, color);
    VdrawVLine(x + width - 1, y, height, color);
}


void VfillRect(int x, int y, int width, int height, unsigned int color) {

    int j, i;
    int endx, endy;

    endx = width + x;
    endy = height + y;

    for(i = y; i < endy; i++) {

        for(j = x; j < endx; j++) {

            plotPixel(j, i, color);
        }
    }
}


void drawCharacter(unsigned char c, int x, int y, unsigned int color) {

    int j, i;
    unsigned char line;
    c = c & 0x7F; //Reduce to base ASCII set

    for(i = 0; i < 12; i++) {

        line = font_array[i * 128 + c];
        for(j = 0; j < 8; j++) {

            if(line & 0x80) plotPixel(x + j, y + i, color);
            line = line << 1;
        }
    }
}


void drawString(unsigned char* s, int x, int y, unsigned int color) {

    while(*s) {

        drawCharacter(*s++, x, y, color);
        x += 8;
    }
}

void VdrawBitmap(int x, int y, bitmap* bmp) {

    int xo, yo;
    unsigned int color;

    //Improvement: If the bitmap isn't using a mask and the screen depth is 32, 
    //then we should use some inline assembly to do a faster REP MOVSD directly 
    //to video RAM. Should significantly improve refresh time

    if(curMode.bpp == 32 && is_linear) {

        for(yo = 0; yo <= (bmp->bottom - bmp->top) && yo < (bmp->height - bmp->top) && yo < (curMode.Yres - y); yo++) {

            memcpy(&bmp->data[((yo + bmp->top) * bmp->width) + bmp->left],
                   &v[((yo + y) * curMode.Xres) + x],
                   (bmp->right - bmp->left + 1) * 4);
        }
    } else {

        //This needs to be corrected so that the bitmap portion is drawn at the cursor location as in the above 32-bit code
        for(yo = bmp->top; yo <= bmp->bottom; yo++) {

            for(xo = bmp->left; xo <= bmp->right; xo++) {
    
                color = bmp->data[yo * bmp->width + xo];

                if(bmp->mask_color && bmp->mask_color == color)
                    continue; 
    
                plotPixel(x + xo, y + yo, bmp->data[yo * bmp->width + xo]);
            }
        }
    }
}

void VcopyScreen(int x, int y, bitmap* bmp) {
    
    //This will break if we're not using a raw framebuffer
    int xo, yo;
    unsigned int* dstbuf = (unsigned int*)(bmp->data);
    
    for(yo = 0; yo < bmp->height; yo++) {

        for(xo = 0; xo < bmp->width; xo++) {

            dstbuf[xo + (yo * bmp->width)] = v[(x + xo) + ((y + yo) * (curMode.pitch/4))];
        }
    }
}
