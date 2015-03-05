#include "../include/p5.h"

#define CMD_COUNT 9

#define RGB(r, g, b) (((r & 0xFF) << 16) + ((g & 0xFF) << 8) + (b & 0xFF))
#define RVAL(x) ((x & 0xFF0000) >> 16)
#define GVAL(x) ((x & 0xFF00) >> 8)
#define BVAL(x) ((x & 0xFF0000) >> 16)

//Function declarations
void usrClear(void);
void consVer(void);
void usrExit(void);
void causeError(void);
void peekV86(void);
void peekKern(void);
void startDos(void);
void sendMsg(void);
void startGui(void);


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

modeInfoBlock curMode;
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
    "GUI"
};

sys_command cmdFunc[CMD_COUNT] = {
    (sys_command)&usrClear,
    (sys_command)&consVer,
    (sys_command)&usrExit,
    (sys_command)&causeError,
    (sys_command)&peekV86,
    (sys_command)&peekKern,
    (sys_command)&sendMsg,
    (sys_command)&startGui
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
    
    prints("Entered console\n");
    while(1) {
        prints("::");
        scans(50, inbuf);
        parse(inbuf);
    }    
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

    postMessage(client_pid, 4, bank_no);
        
    while(!getMessage(&tmp_msg));
}


void startDos(void) {

    client_pid = startV86(":v86.mod");
    if(!client_pid)
        prints("New process could not be started.\n");
}


void plotPixel32(int x, int y, int color) {

    unsigned int* v = (unsigned int*)0xA0000;
    unsigned int linear_pos = y * curMode.pitch + x;
    unsigned int window_pos = linear_pos % (curMode.winsize >> 2);
    unsigned char bank_number = (unsigned char)((linear_pos / (curMode.winsize >> 2)) & 0xFF);
    
    if(bank_number != curBank) {
    
        curBank = bank_number;
        setVESABank(curBank);
    }
    
    v[window_pos] = (unsigned int)((RVAL(color) & 0xFF) << curMode.red_position) | ((GVAL(color) & 0xFF) << curMode.green_position) | ((BVAL(color) & 0xFF) << curMode.blue_position);
}


void plotPixel24(int x, int y, int color) {

    unsigned char* v = (unsigned char*)0xA0000;
    unsigned int linear_pos = y * curMode.pitch + (x * 3);
    unsigned int window_pos = linear_pos % (curMode.winsize / 3);
    unsigned char bank_number = (unsigned char)((linear_pos / (curMode.winsize / 3)) & 0xFF);
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


void plotPixel32(int x, int y, int color) {

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


void drawRect(int x, int y, int height, int width, unsigned int color) {

    int i, j;

    for(j = 0; j < height; j++) {
        
        for(i = 0; i < width; i++) {
        
            plotPixel(i, j, color);
        }
    }
}


void startGui(void) {

    int i;
    unsigned short mode;
    int max = sizeof(ModeInfoBlock);
    unsigned char* wipePtr = (unsigned char*)&curMode;
    
    for(i = 0; i < max; i++)
        wipePtr[i] = 0;
    
    startDos();
    
    if(!(mode = getMode())) {
    
        prints("Could not find a valid VESA mode.\n");
        return;
    }
    
    setMode(mode);
    
    drawRect(0, 0, curMode.Xres, curMode.Yres, RGB(255, 0, 0));
}


ModeInfoBlock* getModeInfo(unsigned short mode) {

    unsigned short seg, off;

    postMessage(client_pid, 2, modeList[i]);
        
    while(!getMessage(&tmp_msg));
    
    seg = (unsigned short)(tmp_msg.payload & 0xFFFF);
    
    while(!getMessage(&tmp_msg));
    
    off = (unsigned short)(tmp_msg.payload & 0xFFFF);
    return (ModeInfoBlock*)0x83000;
}


unsigned short getMode(void) { 

    message tmp_msg;
    unsigned short* modeList;
    ModeInfoBlock* info;
    unsigned short seg, off;
    unsigned short selected_mode = 0;
    unsigned int max_size;
    unsigned short selected_mode;
    int i;
    
    postMessage(client_pid, 1, 0);
    
    while(!getMessage(&tmp_msg));
    
    seg = (unsigned short)(tmp_msg.payload & 0xFFFF);
    
    while(!getMessage(&tmp_msg));
    
    off = (unsigned short)(tmp_msg.payload & 0xFFFF);
    modeList = (unsigned short*)0x82022;
    
    prints("Available mode numbers: \n");
    
    i = 0;
    max_size = 0;
    selected_mode = 0;
    
    while(modeList[i] != 0xFFFF) {
    
        info = getModeInfo(modeList[i]);
        
        if((info->Xres + info->Yres) > max_size && (info->bpp > 8)) {
        
            selected_mode = modeList[i];
            max_size = info->Xres + info->Yres;
        }
        
        prints("   0x"); printHexWord(modeList[i++]); prints(" ("); printHexWord(info->Xres); prints(", "); printHexWord(info->Yres); prints(", "); printHexByte(info->bpp); prints("bpp)\n");
    }
}


void setMode(unsigned short mode) {

    unsigned char* tmp_info, cast_mode;
    int i;
    
    postMessage(client_pid, 3, mode);
    
    //Should include timeouts for message waits like this
    while(!getMessage(&tmp_msg)); 
    
    tmp_info = (unsigned char*)getModeInfo(mode);
    cast_mode = (unsigned char*)curMode;
    
    for(i = 0; i < sizeof(ModeInfoBlock); i++)
        cast_mode[i] = tmp_info[i];
}


void sendMsg(void) {

   message tmp_msg;

   postMessage(client_pid, 0, 0);
   
   while(!getMessage(&tmp_msg));
   
   prints("Message was: "); printHexDword(tmp_msg.payload); prints("\n");
}
