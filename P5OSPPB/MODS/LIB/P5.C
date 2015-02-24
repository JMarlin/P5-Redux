#include "../include/p5.h"


void pchar(char* s) {
    
    __asm__ volatile (
        "mov $0x01, %%eax \n"
        "mov %0, %%ebx \n"
        "int $0xFF \n"
        :
        : "r" (s)
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
        : "r" (c)
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


void scans(unsigned int length, char* outstr) {
    
    unsigned char temp_char;
    int index = 0;
  
    for(index = 0 ; index < length-1 ; ) {    
        temp_char = getch();

        if(temp_char != 0) {       
            outstr[index] = temp_char;
            pchar(outstr[index]);
    
            if(outstr[index] == '\n') {
                outstr[index] = 0;
                break;
            }

            index++;
            
            if(index == length-1)
                pchar('\n');    
        }
    }
    
    outstr[index+1] = 0;
}

