#include "../include/p5.h"


void prints(char* s) {
    
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


void scans(int c, char* b) {

    __asm__ volatile (
        "mov $0x02, %%eax \n"
        "mov %0, %%ebx \n"
        "mov %1, %%ecx \n"
        "int $0xFF \n"
        :
        : "r" (c), "r" (b)
        : "eax", "ebx", "ecx"
    );
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
