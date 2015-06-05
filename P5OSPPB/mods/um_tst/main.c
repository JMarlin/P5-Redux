#include "../include/p5.h"
#include "font.h" //Should eventually make this into a binary font file loadable via the filesystem

#define CMD_COUNT 9

#define RGB(r, g, b) (((r & 0xFF) << 16) + ((g & 0xFF) << 8) + (b & 0xFF))
#define RVAL(x) ((x & 0xFF0000) >> 16)
#define GVAL(x) ((x & 0xFF00) >> 8)
#define BVAL(x) (x & 0xFF)

//Function declarations
void usrClear(void);
void consVer(void);
void usrExit(void);
void causeError(void);
void peekV86(void);
void peekKern(void);
void startDos(void);
void sendMsg(void);
void startGui(unsigned short mode);
void showModes(void);
void enterMode(void);


//Typedefs
typedef void (*sys_command)(void);

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

//Variable declarations
char* cmdWord[CMD_COUNT] = {
    "CLR",
    "VER",
    "EXIT",
    "ERR",
    "V86",
    "KERN",
    "MSG",
    "MODES",
    "SET"
};

sys_command cmdFunc[CMD_COUNT] = {
    (sys_command)&usrClear,
    (sys_command)&consVer,
    (sys_command)&usrExit,
    (sys_command)&causeError,
    (sys_command)&peekV86,
    (sys_command)&peekKern,
    (sys_command)&sendMsg,
    (sys_command)&showModes,
    (sys_command)&enterMode
};

char inbuf[50];


void parse(char* cmdbuf) {

    int i, found;

    found = 0;
    for(i = 0; i < CMD_COUNT; i++) {

        if(strcmp(cmdWord[i], cmdbuf)) {

            found = 1;
            cmdFunc[i]();
            break;
        }
    }

    if(!found) {

        prints("Unknown command ");
        prints(cmdbuf);
        prints("\n");
    }
}

void main(void) {

    //Init the v86 video service thread
    startDos();

    prints("Entered console\n");
    while(1) {
        prints("::");
        scans(50, inbuf);
        parse(inbuf);
    }
}


void enterMode(void) {

    unsigned short modenum = 0;

    prints("What mode number?: ");
    scans(5, inbuf);
    prints("\n");

    modenum = (((inbuf[0] > 9 && inbuf[0] < 0) ? inbuf[0] - 'a' : inbuf[0] - '0') << 12) |
              (((inbuf[1] > 9 && inbuf[1] < 0) ? inbuf[1] - 'a' : inbuf[1] - '0') << 8) |
	      (((inbuf[2] > 9 && inbuf[2] < 0) ? inbuf[2] - 'a' : inbuf[2] - '0') << 4) |
	      (((inbuf[3] > 9 && inbuf[3] < 0) ? inbuf[3] - 'a' : inbuf[3] - '0'));
    startGui(modenum);
}


int strcmp(char* s1, char* s2) {

    int i;

    for(i = 0; s1[i] && s2[i]; i++)
        if(s1[i] != s2[i])
            return 0;

    if(s1[i] != s2[i])
        return 0;

    return 1;
}


void usrClear(void) {

    clearScreen();
}


void consVer(void) {

    prints("P5 usermode console build 1\n");
    prints("P5 build [need fmt print and P5 build number hook]\n");
}


void usrExit(void) {

    terminate();
}


void causeError(void) {

    __asm__ volatile ("cli");
}


void peekV86(void) {

    unsigned char* lowAddr = (unsigned char*)0x7C00;
    unsigned char chkValue;

    chkValue = lowAddr[0];

    if(chkValue < 0x7F) {
        prints("Value is low\n");
    } else {
        prints("Value is high\n");
    }
}


void peekKern(void) {

    unsigned char* hiAddr = (unsigned char*)0x200000;
    unsigned char chkValue;

    chkValue = hiAddr[0];

    if(chkValue < 0x7F) {
        prints("Value is low\n");
    } else {
        prints("Value is high\n");
    }
}


void setVESABank(unsigned char bank_no) {

    message tmp_msg;

    postMessage(client_pid, 4, bank_no);

    while(!getMessage(&tmp_msg));
}


void startDos(void) {

    client_pid = startV86(":v86.mod");
    if(!client_pid)
        prints("New process could not be started.\n");
}


ModeInfoBlock* getModeInfo(unsigned short mode) {

    message tmp_msg;
    unsigned short seg, off;

    postMessage(client_pid, 2, mode);

    while(!getMessage(&tmp_msg));

    off = (unsigned short)(tmp_msg.payload & 0xFFFF);

    while(!getMessage(&tmp_msg));

    seg = (unsigned short)(tmp_msg.payload & 0xFFFF);
    return (ModeInfoBlock*)0x83000; //Should NOT do this
}


unsigned short getMode(void) {

    message tmp_msg;
    unsigned short* modeList;
    ModeInfoBlock* info;
    VESAInfo* vinfo;
    unsigned short seg, off;
    unsigned short selected_mode = 0;
    unsigned int max_size;
    int i;

    postMessage(client_pid, 1, 0);

    while(!getMessage(&tmp_msg));

    seg = (unsigned short)(tmp_msg.payload & 0xFFFF);

    //This one is just for sync
    postMessage(client_pid, 1, 0);

    while(!getMessage(&tmp_msg));

    off = (unsigned short)(tmp_msg.payload & 0xFFFF);
    vinfo = (VESAInfo*)((((unsigned int)seg) << 4) + off);
    prints("VESA Info at 0x"); printHexDword((unsigned int)vinfo); prints(":\n");
    prints("    sig: "); pchar(vinfo->sig[0]); pchar(vinfo->sig[1]); pchar(vinfo->sig[2]); pchar(vinfo->sig[3]); pchar('\n');
    prints("    version: "); printHexWord(vinfo->version); pchar('\n');
    prints("    OEMStringPtr: "); printHexWord(vinfo->OEMStringPtrSeg); pchar(':'); printHexWord(vinfo->OEMStringPtrOff); pchar('\n');
    prints("    Capabilities: "); printHexByte(vinfo->capabilities[0]); printHexByte(vinfo->capabilities[1]); printHexByte(vinfo->capabilities[2]); printHexByte(vinfo->capabilities[3]); pchar('\n');
    prints("    Modes ptr: "); printHexWord(vinfo->modePtrSeg); pchar(':'); printHexWord(vinfo->modePtrOff); pchar('\n');
    prints("    Memsize: "); printHexWord(vinfo->memSize); pchar('\n');
    prints("    Reserved: "); printHexByte(vinfo->reserved); pchar('\n');
    prints("Press enter to continue.\n");
    scans(5, inbuf);
    modeList = (unsigned short*)((((unsigned int)vinfo->modePtrSeg) << 4) + vinfo->modePtrOff);
    prints("Available mode numbers: (from address 0x"); printHexDword((unsigned int)modeList); pchar('\n');
    i = 0;
    max_size = 0;
    selected_mode = 0;

    while(modeList[i] != 0xFFFF) {

        info = getModeInfo(modeList[i]);

        if((info->Xres + info->Yres) > max_size && (info->bpp >= 24) && (info->attributes & 0x1)) {

            selected_mode = modeList[i];
            max_size = info->Xres + info->Yres;
        }

        prints("   0x"); printHexWord(modeList[i]); prints(" ("); printHexWord(info->Xres); prints(", "); printHexWord(info->Yres); prints(", "); printHexByte(info->bpp); prints("bpp)\n");
        i++;

        if(!(i % 20)) {

            prints("Press enter to continue listing...\n");
            scans(5, inbuf);
        }
    }

    prints("Selected mode number: "); printHexWord(selected_mode); pchar('\n');

    return selected_mode;
}


int setMode(unsigned short mode) {

    message tmp_msg;
    unsigned char *tmp_info, *cast_mode;
    int i;

    prints("Sending mode change message...");
    postMessage(client_pid, 3, mode);
    prints("listening...");

    //Should include timeouts for message waits like this
    while(!getMessage(&tmp_msg));
    prints("done\n");

    if((tmp_msg.payload & 0xFF) == 0x4F) {

        if(tmp_msg.payload & 0x0100) {

            prints("Set mode command failed.\n");
            return 0;
        }
    } else {

        prints("Set mode command not supported.\n");
        return 0;
    }

    tmp_info = (unsigned char*)getModeInfo(mode);
    cast_mode = (unsigned char*)&curMode;

    for(i = 0; i < sizeof(ModeInfoBlock); i++)
        cast_mode[i] = tmp_info[i];

    return 1;
}


void plotPixel32(int x, int y, int color) {

    unsigned int* v = (unsigned int*)0xA0000;
    unsigned int linear_pos = x + (y * (curMode.pitch/4));
    unsigned int window_pos = linear_pos % (curMode.winsize * 0x100);
    unsigned char bank_number = (unsigned char)((linear_pos / (curMode.winsize * 0x100)) & 0xFF);

    if(bank_number != curBank) {

        curBank = bank_number;
        setVESABank(curBank);
    }

    v[window_pos] = (unsigned int)((RVAL(color) & 0xFF) << curMode.red_position) | ((GVAL(color) & 0xFF) << curMode.green_position) | ((BVAL(color) & 0xFF) << curMode.blue_position);
}


void plotPixel24(int x, int y, int color) {

    unsigned char* v = (unsigned char*)0xA0000;
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

    unsigned short* v = (unsigned short*)0xA0000;
    unsigned int linear_pos = y * curMode.pitch + x;
    unsigned int window_pos = linear_pos % (curMode.winsize >> 1);
    unsigned char bank_number = (unsigned char)((linear_pos / (curMode.winsize >> 1)) & 0xFF);

    if(bank_number != curBank) {

        curBank = bank_number;
        setVESABank(curBank);
    }

    v[window_pos] = (unsigned short)((RVAL(color) & 0xF) << curMode.red_position) | ((GVAL(color) & 0xF) << curMode.green_position) | ((BVAL(color) & 0xF) << curMode.blue_position);
}


void plotPixel(int x, int y, unsigned int color) {

    if(curMode.bpp == 32) plotPixel32(x, y, color);
    if(curMode.bpp == 24) plotPixel24(x, y, color);
    if(curMode.bpp == 16) plotPixel16(x, y, color);
}


void drawHLine(int x, int y, int length, unsigned int color) {

    int i, endx;

    endx = x + length;

    for(i = x; i < endx; i++)
        plotPixel(i, y, color);
}

void drawVLine(int x, int y, int length, unsigned int color) {

    int i, endy;

    endy = length + y;

    for(i = y; i < endy; i++)
        plotPixel(x, i, color);
}


void drawRect(int x, int y, int width, int height, unsigned int color) {

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


void drawPanel(int x, int y, int width, int height, unsigned int color, int border_width, int invert) {

    unsigned char r = RVAL(color);
    unsigned char g = GVAL(color);
    unsigned char b = BVAL(color);
    unsigned int light_color = RGB(r > 195 ? 255 : r + 60, g > 195 ? 255 : g + 60, b > 195 ? 255 : b + 60);
    unsigned int shade_color = RGB(r < 60 ? 0 : r - 60, g < 60 ? 0 : g - 60, b < 60 ? 0 : b - 60);
    unsigned int temp;
    int i;

    if(invert) {

        temp = shade_color;
        shade_color = light_color;
        light_color = temp;
    }

    for(i = 0; i < border_width; i++) {

        //Top edge
        drawHLine(x+i, y+i, width-(2*i), light_color);

        //Bottom edge
        drawHLine(x+i, (y+height)-(i+1), width-(2*i), shade_color);

        //Left edge
        drawVLine(x+i, y+i+1, height-((i+1)*2), light_color);

        //Right edge
        drawVLine(x+width-i-1, y+i+1, height-((i+1)*2), shade_color);
    }

    //Fill
    drawRect(x+border_width, y+border_width, width-(2*border_width), height-(2*border_width), color);
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


drawCharacterBold(unsigned char c, int x, int y, unsigned int color) {

    drawCharacter(c, x, y, color);
    drawCharacter(c, x+1, y, color);
    drawCharacter(c, x, y+1, color);
    drawCharacter(c, x+1, y+1, color);
}


void drawString(unsigned char* s, int x, int y, unsigned int color) {

    while(*s) {

        drawCharacter(*s++, x, y, color);
        x += 8;
    }
}


void drawButton(unsigned char* text, int x, int y, int width, int height, unsigned int color, int border_width) {

    int str_len, lines, cols, lmargin, tmargin, l, c, ocols;

    for(str_len = 0; text[str_len]; str_len++);

    lines = (str_len * 8) / (width - border_width*2);

    //Do a ceiling
    if((str_len*8) % width)
        lines++;

    cols = (width - border_width*2) / 8;
    ocols = cols;
    lmargin = (width - border_width*2 - (cols*8)) / 2;
    tmargin = (height - border_width*2 - (lines*12)) / 2;

    drawPanel(x, y, width, height, color, border_width, 0);

    for(l = 0; l < lines; l++) {

        //Recalculate values for the last lines in case it's
        //shorter than the rest
        if(l == (lines - 1)) {

            cols = str_len - (cols * (lines - 1));
            lmargin = (width - border_width*2 - (cols*8)) / 2;
        }

        for(c = 0; c < cols; c++) {

            drawCharacterBold(text[c+(l*ocols)], x + lmargin + border_width + (c*8), y +  border_width + tmargin + (l*12), 0);
        }
    }


}


void showModes(void) {

    getMode();
}


int cmd_x = 0;
int cmd_y = 0;

void cmd_putc(unsigned char c) {


}


void startGui(unsigned short mode) {

    int i, x, y, max_chars, os_build;
    unsigned char tmpch;
    unsigned char *tmp_info, *cast_mode;
    int max = sizeof(ModeInfoBlock);
    unsigned char* wipePtr = (unsigned char*)&curMode;

    for(i = 0; i < max; i++)
        wipePtr[i] = 0;

    tmp_info = (unsigned char*)getModeInfo(mode);
    cast_mode = (unsigned char*)&curMode;

    for(i = 0; i < sizeof(ModeInfoBlock); i++)
        cast_mode[i] = tmp_info[i];

    if(!setMode(mode)) {

        prints("Mode number 0x");
	printHexWord(mode);
	prints(" unsupported.\n");
        return;
    }

    //Backdrop
    drawRect(0, 0, curMode.Xres, curMode.Yres, RGB(11, 162, 193));

    //System Bar
    drawPanel(curMode.Xres-60, 0, 60, curMode.Yres, RGB(200, 200, 200), 2, 0);

    //Indent for fun
    drawButton("P5OS", curMode.Xres-55, 5, 50, 50, RGB(200, 200, 200), 2);

    //Redundant text for shadow
    drawString("Build #", 1, 1, 0);
    os_build = getBuildNumber();
    drawCharacter((os_build >> 28 & 0xF) > 9 ? (os_build >> 28 & 0xF) + 'A' : (os_build >> 28 & 0xF) + '0', 57, 1, 0);
    drawCharacter((os_build >> 24 & 0xF) > 9 ? (os_build >> 24 & 0xF) + 'A' : (os_build >> 24 & 0xF) + '0', 65, 1, 0);
    drawCharacter((os_build >> 20 & 0xF) > 9 ? (os_build >> 20 & 0xF) + 'A' : (os_build >> 20 & 0xF) + '0', 73, 1, 0);
    drawCharacter((os_build >> 16 & 0xF) > 9 ? (os_build >> 16 & 0xF) + 'A' : (os_build >> 16 & 0xF) + '0', 81, 1, 0);
    drawCharacter((os_build >> 12 & 0xF) > 9 ? (os_build >> 12 & 0xF) + 'A' : (os_build >> 12 & 0xF) + '0', 89, 1, 0);
    drawCharacter((os_build >> 8 & 0xF) > 9 ? (os_build >> 8 & 0xF) + 'A' : (os_build >> 8 & 0xF) + '0', 97, 0, 1);
    drawCharacter((os_build >> 4 & 0xF) > 9 ? (os_build >> 4 & 0xF) + 'A' : (os_build >> 4 & 0xF) + '0', 105, 1, 0);
    drawCharacter((os_build & 0xF) > 9 ? (os_build & 0xF) + 'A' : (os_build & 0xF) + '0', 113, 1, 0);

    //Text test
    drawString("Build #", 0, 0, RGB(255, 255, 255));
    os_build = getBuildNumber();
    drawCharacter((os_build >> 28 & 0xF) > 9 ? (os_build >> 28 & 0xF) + 'A' : (os_build >> 28 & 0xF) + '0', 56, 0, RGB(255, 255, 255));
    drawCharacter((os_build >> 24 & 0xF) > 9 ? (os_build >> 24 & 0xF) + 'A' : (os_build >> 24 & 0xF) + '0', 64, 0, RGB(255, 255, 255));
    drawCharacter((os_build >> 20 & 0xF) > 9 ? (os_build >> 20 & 0xF) + 'A' : (os_build >> 20 & 0xF) + '0', 72, 0, RGB(255, 255, 255));
    drawCharacter((os_build >> 16 & 0xF) > 9 ? (os_build >> 16 & 0xF) + 'A' : (os_build >> 16 & 0xF) + '0', 80, 0, RGB(255, 255, 255));
    drawCharacter((os_build >> 12 & 0xF) > 9 ? (os_build >> 12 & 0xF) + 'A' : (os_build >> 12 & 0xF) + '0', 88, 0, RGB(255, 255, 255));
    drawCharacter((os_build >> 8 & 0xF) > 9 ? (os_build >> 8 & 0xF) + 'A' : (os_build >> 8 & 0xF) + '0', 96, 0, RGB(255, 255, 255));
    drawCharacter((os_build >> 4 & 0xF) > 9 ? (os_build >> 4 & 0xF) + 'A' : (os_build >> 4 & 0xF) + '0', 104, 0, RGB(255, 255, 255));
    drawCharacter((os_build & 0xF) > 9 ? (os_build & 0xF) + 'A' : (os_build & 0xF) + '0', 112, 0, RGB(255, 255, 255));

    //body of 'window'
    drawPanel(20, 20, curMode.Xres-100, curMode.Yres-40, RGB(200, 200, 200), 2, 0);

    //'window' inner bevel
    drawPanel(25, 25, curMode.Xres-110, curMode.Yres-50, RGB(200, 200, 200), 2, 1);

    //'terminal' background
    drawRect(27, 27, curMode.Xres-114, curMode.Yres-54, 0);

    //Simple input loop
    x = 2;
    y = 0;
    max_chars = (curMode.Xres-114)/8;

    drawString("::", 27, 27, RGB(0, 255, 0));

    while(1) {

        //Wait for keypress
        while(!(tmpch = getch()));

        if(tmpch == '\n') {

            x = 2;
            y++;
            drawString("::", 27, (y*12)+27, RGB(0, 255, 0));
        } else {

            drawCharacter(tmpch, (x*8)+27, (y*12)+27, RGB(0, 255, 0));
            x++;

            if(x > max_chars) {

                x = 0;
                y++;
            }
        }
    }

    //drawRect(102, 102, curMode.Xres - 204, curMode.Yres - 204, RGB(68, 76, 82));
    /*
    int row;
    int inner_height = (curMode.Yres - 204);
    int change_rate = inner_height / 255;
    int shade_val = 255;
    for(row = 0; row < inner_height; row++) {

        drawHLine(102, row + 102, curMode.Xres - 204, RGB(0, 0, shade_val));

        if(!(row % change_rate))
            shade_val--;
    }
    */
}

void sendMsg(void) {

   message tmp_msg;

   postMessage(client_pid, 0, 0);

   while(!getMessage(&tmp_msg));

   prints("Message was: "); printHexDword(tmp_msg.payload); prints("\n");
}
