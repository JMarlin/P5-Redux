#include "../include/p5.h"


void pchar(char c) {
    
    unsigned int ec = (unsigned int)c;
    
    __asm__ volatile (
        "mov $0x01, %%eax \n"
        "mov %0, %%ebx \n"
        "int $0xFF \n"
        :
        : "r" (ec)
        : "eax", "ebx"
    );
}


void terminate(void) {
    
    __asm__ volatile (
        "mov $0x00, %%eax \n"
        "int $0xFF \n"
        :
        :
        : "eax"
    );
}


unsigned char getch() {

    unsigned int c;

    __asm__ volatile (
        "mov $0x02, %%eax \n"
        "int $0xFF \n"
        "mov %%ebx, %0"
        : "=r" (c)
        : 
        : "ebx"
    );
    
    return (unsigned char)(c & 0xFF);
}


void clearScreen() {

    __asm__ volatile (
        "mov $0x03, %%eax \n"
        "int $0xFF \n"
        :
        :
        : "eax"
    );
}


void startProc() {

    __asm__ volatile (
        "mov $0x04, %%eax \n"
        "int $0xFF \n"
        :
        :
        : "eax"
    );
}


void nextProc() {

    __asm__ volatile (
        "mov $0x05, %%eax \n"
        "int $0xFF \n"
        :
        :
        : "eax"
    );
}


void endProc() {

    __asm__ volatile (
        "mov $0x06, %%eax \n"
        "int $0xFF \n"
        :
        :
        : "eax"
    );
}


unsigned int getTimeCounter() {

    unsigned int c;

    __asm__ volatile (
        "mov $0x07, %%eax \n"
        "int $0xFF \n"
        "mov %%ebx, %0"
        : "=r" (c)
        : 
        : "ebx"
    );
    
    return c;
}


void scans(int c, char* b) {
    
    unsigned char temp_char;
    int index = 0;
  
    for(index = 0 ; index < c-1 ; ) {    
        temp_char = getch();

        if(temp_char != 0) {       
            b[index] = temp_char;
            pchar(b[index]);
    
            if(b[index] == '\n') {
                b[index] = 0;
                break;
            }

            index++;
            
            if(index == c-1)
                pchar('\n');    
        }
    }
    
    b[index+1] = 0;
}


void prints(char* s) {      
    
    int index = 0;

    while(s[index] != 0) {
        pchar(s[index]);
        index++;
    }
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


