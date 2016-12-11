#include "ascii_o.h"
#include "ascii_i.h"
#include "../core/util.h"
#include "serial.h"
#include "../process/process.h"
#include "../core/syscall.h"
#include "keyboard.h"

#ifdef KPRINTS_ON
#define LINECOUNT 24
#define KP_COLOR 0x74
int kcursor = 0;
#else 
#define LINECOUNT 25
#endif

char* screenBase = 0;
int cursor_x = 0, cursor_y = 0;
char color_code = 0x07;

void ramdump(unsigned int address, unsigned int count) {

    int i = 0;
    char* ram = (char*)address;
    char* end = (char*)(address+count);

    while(ram < end) {

        pchar('\n');
        printHexDword((int)ram);
        pchar(':');
        pchar(' ');

        for(i = 0; i < 16 && ram < end; i++, ram++) {

            printHexByte(*ram);
            pchar(' ');
        }

        pchar('|');
        pchar(' ');

        for(i = -i; i < 0; i++)
            pchar(ram[i]);
    }

    while(1);
}


void clear(void) {

    int i;

    for(i = 0; i < LINECOUNT * 160; i += 2) 
        screenBase[i] = ' ';

    setCursor(0, 0);
}


unsigned char digitToHex(unsigned char digit) {

    if(digit < 0xA) {
        return (digit + '0');
    } else {
        return ((digit - 0xA) + 'A');
    }
}


void printHexByte(unsigned char byte) {

    pchar(digitToHex((byte & 0xF0)>>4));
    pchar(digitToHex(byte & 0xF));
}


void printHexWord(unsigned short wd) {

    printHexByte((unsigned char)((wd & 0xFF00)>>8));
    printHexByte((unsigned char)(wd & 0xFF));
}


void printHexDword(unsigned int dword) {

    printHexWord((unsigned short)((dword & 0xFFFF0000)>>16));
    printHexWord((unsigned short)(dword & 0xFFFF));
}

void (*tm_cb)(void);

void textModeFinish(unsigned int a, unsigned int b, unsigned int c) {
    
    keyboard_init();
    setupKeyTable();
    tm_cb();
}

//THIS IS FOR FORCING THE SYSTEM BACK TO TEXT MODE IN THE EVENT OF AN EMERGENCY
//IT OVERRIDES ALL PROCESS MANAGEMENT, SO ONLY USE IF THE SYSTEM HAS FAILED
void enterTextMode(void (*cb)(void)) {
    
    char* usrCode = (char*)0x80000;
    tm_cb = cb;
        
    //Put the code into the v86 code area
    resetProcessCounter();
    set_call_zero_cb(&textModeFinish); //Make the interrupt 

    //Do INT 0x10
    usrCode[0] = 0xB0; // -|
    usrCode[1] = 0x03; // -\_mov al, 0x03 (80x25 16-color text)
    usrCode[2] = 0xCD; // -|
    usrCode[3] = 0x10; // -\_int 0x10
    //Do INT 0xFF #0, return to kernel init
    usrCode[4] = 0x31; //  |
    usrCode[5] = 0xC0; // -\_xor ax, ax 
    usrCode[6] = 0xCD; // -|
    usrCode[7] = 0xFF; // -\_int 0xff

    //Execute the loaded 16-bit code
    enterProc(exec_loaded_v86(100));
}



void initScreen() {

    int i;

    initSerial();
    screenBase = (char*)0xB8000;
    setCursor(0, 0);
    
    for(i = 0; i < LINECOUNT* 160; i += 2) {
    
        screenBase[i] = ' ';
        screenBase[i + 1] = color_code;
    }
        
#ifdef KPRINTS_ON
    for(; i < (LINECOUNT + 1) * 160; i += 2) {
        
        screenBase[i] = ' ';
        screenBase[i + 1] = KP_COLOR;
    }
#endif
    
    return;
}


void setCursor(int x, int y) {

    int linear_location = (y*80)+x;
    unsigned short* video_port = (unsigned short*)0x0463;

    cursor_x = x;
    cursor_y = y;
    outb(video_port[0], 0x0F);
    outb(video_port[0]+1, (unsigned char)(linear_location&0xFF));
    outb(video_port[0], 0x0E);
    outb(video_port[0]+1, (unsigned char)((linear_location>>8)&0xFF));
    return;
}


void setColor(char newCode) {

    color_code = newCode;
    return;
}


void scrollScreen() {

    int x, y;

    for(y = 0; y < LINECOUNT - 1; y++) {

        for(x = 0; x < 160; x+=2) {
            screenBase[(y*160)+x] = screenBase[((y+1)*160)+x];
        }
    }

    for(x = 0; x < 160; x++) {
        screenBase[((LINECOUNT - 1)*160)+(x++)] = 0x00;
    }

    setCursor(0, LINECOUNT - 1);
    return;
}

void pchar(char _inin) {

    serPutch(_inin);
    if(_inin != '\n'){

        //Insert the character
        screenBase[((cursor_y*80)+cursor_x)*2] = _inin;
        cursor_x++;
    }

    if(cursor_x == 80 || _inin == '\n') {
        cursor_x = 0;
        cursor_y++;

        if(cursor_y == LINECOUNT){
            scrollScreen();
        }
    }

    setCursor(cursor_x, cursor_y);
    return;
}


void prints(char* _str) {

    int index = 0;

    while(_str[index] != 0) {
        pchar(_str[index]);
        index++;
    }
}

#ifdef KPRINTS_ON

void kpchar(unsigned char c) {
    
    int i;
    
    if(kcursor == 80 || c == '\n') {
        
        kcursor = 0;
        
        for(i = LINECOUNT * 160; i < (LINECOUNT + 1) * 160; i += 2)
            screenBase[i] = ' ';
    }
    
    if(c != '\n'){

        //Insert the character
        screenBase[(LINECOUNT * 160) + (kcursor*2)] = c;
        kcursor++;
    }
    
    return;
}

void kprints(unsigned char* s) {
    
    int index = 0;

    while(s[index] != 0) {
        kpchar(s[index]);
        index++;
    }
}

void kprintHexByte(unsigned char byte) {

    kpchar(digitToHex((byte & 0xF0)>>4));
    kpchar(digitToHex(byte & 0xF));
}


void kprintHexWord(unsigned short wd) {

    kprintHexByte((unsigned char)((wd & 0xFF00)>>8));
    kprintHexByte((unsigned char)(wd & 0xFF));
}


void kprintHexDword(unsigned int dword) {

    kprintHexWord((unsigned short)((dword & 0xFFFF0000)>>16));
    kprintHexWord((unsigned short)(dword & 0xFFFF));
}

#endif
