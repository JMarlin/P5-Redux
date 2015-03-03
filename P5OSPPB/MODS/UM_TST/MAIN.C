#include "../include/p5.h"

#define CMD_COUNT 10

unsigned int client_pid = 0;

//Function declarations
void usrClear(void);
void consVer(void);
void usrExit(void);
void causeError(void);
void peekV86(void);
void peekKern(void);
void startDos(void);
void sendMsg(void);
void getModes(void);
void setMode(void);


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
  unsigned short pitch; // bytes per scanline
 
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

//Variable declarations
char* cmdWord[CMD_COUNT] = {
    "CLR",
    "VER",
    "EXIT",
    "ERR",
    "V86",
    "KERN",
    "START",
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
    (sys_command)&startDos,
    (sys_command)&sendMsg,
    (sys_command)&getModes,
    (sys_command)&setMode
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


void startDos(void) {

    client_pid = startV86(":v86.mod");
    if(!client_pid)
        prints("New process could not be started.\n");
}


void getModes(void) { 

    message tmp_msg;
    unsigned short* modeList;
    ModeInfoBlock* info;
    unsigned short seg, off;
    int i;
    
    postMessage(client_pid, 1, 0);
    
    while(!getMessage(&tmp_msg));
    
    seg = (unsigned short)(tmp_msg.payload & 0xFFFF);
    
    while(!getMessage(&tmp_msg));
    
    off = (unsigned short)(tmp_msg.payload & 0xFFFF);
    modeList = (unsigned short*)0x82022;
    
    prints("Available mode numbers: \n");
    
    i = 0;
    while(modeList[i] != 0xFFFF) {
    
        postMessage(client_pid, 2, modeList[i]);
        
        while(!getMessage(&tmp_msg));
    
        seg = (unsigned short)(tmp_msg.payload & 0xFFFF);
    
        while(!getMessage(&tmp_msg));
    
        off = (unsigned short)(tmp_msg.payload & 0xFFFF);
        info = (ModeInfoBlock*)0x83000;
        prints("   0x"); printHexWord(modeList[i++]); prints(" ("); printHexWord(info->Xres); prints(", "); printHexWord(info->Yres); prints(", "); printHexByte(info->bpp); prints("bpp)\n");
    }
}


void setMode(void) {

    int i, strlen;
    unsigned short convNumber;
    message tmp_msg;

    prints("Mode number: 0x");
    scans(6, inbuf);
    convNumber = (inbuf[3] >= 'A' ? inbuf[3] - 'A' : inbuf[3] - '0') +
                 ((inbuf[2] >= 'A' ? inbuf[2] - 'A' : inbuf[2] - '0') << 4) +
                 ((inbuf[1] >= 'A' ? inbuf[1] - 'A' : inbuf[1] - '0') << 8) +
                 ((inbuf[0] >= 'A' ? inbuf[0] - 'A' : inbuf[0] - '0') << 12);
    postMessage(client_pid, 3, convNumber);
    while(!getMessage(&tmp_msg)); 
}


void sendMsg(void) {

   message tmp_msg;

   postMessage(client_pid, 0, 0);
   
   while(!getMessage(&tmp_msg));
   
   prints("Message was: "); printHexDword(tmp_msg.payload); prints("\n");
}
