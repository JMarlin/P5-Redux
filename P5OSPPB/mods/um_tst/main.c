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
    unsigned char *tmp_info, *cast_mode;
    int max = sizeof(ModeInfoBlock);
    unsigned char* wipePtr = (unsigned char*)&curMode;
    
    for(i = 0; i < max; i++)
        wipePtr[i] = 0;
    
    startDos();
    
    if(!(mode = getMode())) {
    
        prints("Could not find a valid VESA mode.\n");
        return;
    }
    
    prints("Press enter to continue...\n");
    scans(5, inbuf);
    
    tmp_info = (unsigned char*)getModeInfo(mode);
    cast_mode = (unsigned char*)&curMode;
    
    for(i = 0; i < sizeof(ModeInfoBlock); i++)
        cast_mode[i] = tmp_info[i];
    
    prints("\nInfo block for VESA mode 0x"); printHexWord(mode); prints(":\n");
    prints("    attributes: 0x"); printHexWord(curMode.attributes); prints("\n");
    prints("    winA: 0x"); printHexByte(curMode.winA); prints("\n");
    prints("    winB: 0x"); printHexByte(curMode.winB); prints("\n");
    prints("    granularity: 0x"); printHexWord(curMode.granularity); prints("\n");
    prints("    winsize: 0x"); printHexWord(curMode.winsize); prints("\n");
    prints("    segmentA: 0x"); printHexWord(curMode.segmentA); prints("\n");
    prints("    segmentB: 0x"); printHexWord(curMode.segmentB); prints("\n");
    prints("    realFctPtrSeg: 0x"); printHexWord(curMode.realFctPtrSeg); prints("\n"); 
    prints("    realFctPtrOff: 0x"); printHexWord(curMode.realFctPtrOff); prints("\n");  
    prints("    pitch: 0x"); printHexWord(curMode.pitch); prints("\n");
    prints("    Xres: 0x"); printHexWord(curMode.Xres); prints("\n");
    prints("    Yres: 0x"); printHexWord(curMode.Yres); prints("\n");
    prints("    Wchar: 0x"); printHexByte(curMode.Wchar); prints("\n");
    prints("    Ychar: 0x"); printHexByte(curMode.Ychar); prints("\n");
    prints("    planes: 0x"); printHexByte(curMode.planes); prints("\n");
    prints("    bpp: 0x"); printHexByte(curMode.bpp); prints("\n");
    prints("    banks: 0x"); printHexByte(curMode.banks); prints("\n");
    prints("    memory_model: 0x"); printHexByte(curMode.memory_model); prints("\n");
    prints("    bank_size: 0x"); printHexByte(curMode.bank_size); prints("\n");
    prints("Press enter to continue...\n");
    scans(5, inbuf);
    prints("    image_pages: 0x"); printHexByte(curMode.image_pages); prints("\n");
    prints("    reserved0: 0x"); printHexByte(curMode.reserved0); prints("\n");
    prints("    red_mask: 0x"); printHexByte(curMode.red_mask); prints("\n");
    prints("    red_position: 0x"); printHexByte(curMode.red_position); prints("\n");
    prints("    green_mask: 0x"); printHexByte(curMode.green_mask); prints("\n");
    prints("    green_position: 0x"); printHexByte(curMode.green_position); prints("\n");
    prints("    blue_mask: 0x"); printHexByte(curMode.blue_mask); prints("\n");
    prints("    blue_position: 0x"); printHexByte(curMode.blue_position); prints("\n");
    prints("    rsv_mask: 0x"); printHexByte(curMode.rsv_mask); prints("\n");
    prints("    rsv_position: 0x"); printHexByte(curMode.rsv_position); prints("\n");
    prints("    directcolor_attributes: 0x"); printHexByte(curMode.directcolor_attributes); prints("\n");
    prints("    physbase: 0x"); printHexDword(curMode.physbase); prints("\n");
    prints("    reserved1: 0x"); printHexDword(curMode.reserved1); prints("\n");
    prints("    reserved2: 0x"); printHexWord(curMode.reserved2); prints("\n");
    prints("Press enter to continue...\n");
    scans(5, inbuf);
    
    if(!setMode(mode)) return;
    
    drawRect(0, 0, curMode.Xres, curMode.Yres, RGB(255, 0, 0));
}


void sendMsg(void) {

   message tmp_msg;

   postMessage(client_pid, 0, 0);
   
   while(!getMessage(&tmp_msg));
   
   prints("Message was: "); printHexDword(tmp_msg.payload); prints("\n");
}
