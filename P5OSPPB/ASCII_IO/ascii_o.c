#include "ascii_o.h"
#include "../core/commands.h"
#include "../core/util.h"


char* screenBase;
int cursor_x, cursor_y;
char color_code;


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


void initScreen() {

    color_code = 0x07;
    screenBase = (char*)0xB8000;
    setCursor(0, 0);
    return;
}


//This is probably fucking up our usermode console,
//making it not do shit to make sure of that
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

    for(y = 0; y < 24; y++) {
    
        for(x = 0; x < 160; x+=2) {
            screenBase[(y*160)+x] = screenBase[((y+1)*160)+x];       
        }
    }

    for(x = 0; x < 160; x++) {
        screenBase[(24*160)+(x++)] = 0x00;
        screenBase[(24*160)+x] = color_code;
    }

    //setCursor(0, 24);
    return;
}

void pchar(char _inin) {
    
    if(_inin != '\n'){
    
         //Insert the character
        screenBase[((cursor_y*80)+cursor_x)*2] = _inin;
        screenBase[(((cursor_y*80)+cursor_x)*2)+1] = color_code;
            cursor_x++;
    }

    if(cursor_x == 80 || _inin == '\n') {
        cursor_x = 0;
        cursor_y++;
        
        if(cursor_y == 25){
            scrollScreen();
        }
    }

    //setCursor(cursor_x, cursor_y);
    return;
}


void prints(char* _str) {      
    
    int index = 0;

    while(_str[index] != 0) {
        pchar(_str[index]);
        index++;
    }
}
