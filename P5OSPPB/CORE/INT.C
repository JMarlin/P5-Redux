#include "int.h" 
#include "expt.h"
#include "../ascii_io/ascii_o.h"
#include "../core/global.h"
#include "syscall.h"
#include "../process/process.h"


unsigned char* idtBase = (unsigned char*)0x201000;

//ends 0x201810
unsigned char* idtEntries = (unsigned char*)0x201010;  


void genericInterrupt(void) {

    __asm__ ("cli");
    prints("UNHANDLED INTERRUPT");
    __asm__ ("sti");

    //while(1);
}


void set_idt(char* ptr) {

    DEBUG("Setting idt to "); DEBUG_HD((unsigned int)ptr); DEBUG("\n");
    __asm__ (
        "lidt (%0)\n"
        :  
        : "p" (ptr)
    );
}


void installInterrupt(unsigned char number, intHandler handler, unsigned char dpl) {

    unsigned char* entryBase = ((unsigned char*)idtEntries)+(8*number);
    unsigned short* offset_1 = (unsigned short*)entryBase;
    unsigned short* selector = (unsigned short*)(entryBase+2);
    unsigned char* zero = (unsigned char*)(entryBase+4);
    unsigned char* type_attr = (unsigned char*)(entryBase+5); 
    unsigned short* offset_2 = (unsigned short*)(entryBase+6);

    offset_1[0] = (unsigned short)(((unsigned int)handler) & 0xFFFF);
    
    //The global code segment set up by the bootloader
    selector[0] = 0x8; 
    zero[0] = 0x0;
    
    //0x00 = not present, minimum priviledge ring 00b, storage segment 0
    type_attr[0] = 0x8E | ((dpl & 0x3 ) << 5);  
    offset_2[0] = (unsigned short)(((unsigned int)handler) >> 16);
}


void printIdt(unsigned char number) {

    unsigned char* entryBase = ((unsigned char*)idtEntries)+(8*number);
    unsigned short* offset_1 = (unsigned short*)entryBase;
    unsigned short* selector = (unsigned short*)(entryBase+2);
    unsigned char* zero = (unsigned char*)(entryBase+4);
    unsigned char* type_attr = (unsigned char*)(entryBase+5); 
    unsigned short* offset_2 = (unsigned short*)(entryBase+6);
    
    prints("\nHandler #"); printHexByte(number); prints(" @ "); printHexDword((unsigned int)entryBase);
    prints("\n   offset_1: "); printHexWord(offset_1[0]);
    prints("\n   selector: "); printHexWord(selector[0]);
    prints("\n   zero: "); printHexByte(zero[0]);
    prints("\n   type_attr: "); printHexByte(type_attr[0]); 
    prints("\n   offset_2: "); printHexWord(offset_2[0]);
}


void blankInterrupt(unsigned char number) {

    installInterrupt(number, &genericInterrupt, 0);
}


void initIDT() {

    int i;       
    unsigned short* count = (unsigned short*)idtBase;
    unsigned int* ptr = (unsigned int*)(idtBase + 2); 

    DEBUG("\nSetting idtBase.limit\n");
    count[0] = 0x800;
    DEBUG("Setting idtBase.base\n");
    ptr[0] = (unsigned int)idtEntries;
    DEBUG("Blanking interrupts\n");
    
    for(i = 0; i < 256; i++) {
        blankInterrupt(i);
    }                                                 

    DEBUG("Installing idt\n");
    set_idt(idtBase);
}


void installExceptionHandlers() {

    //All of the default exceptions
    installInterrupt(0x0, &expt_zeroDivide, 0);
    installInterrupt(0x1, &expt_debugCall, 0);
    installInterrupt(0x2, &expt_NMI, 0);
    installInterrupt(0x3, &expt_breakpoint, 3);
    installInterrupt(0x4, &expt_overflow, 0);
    installInterrupt(0x5, &expt_outOfBound, 0);
    installInterrupt(0x6, &expt_illegalOpcode, 0);
    installInterrupt(0x7, &expt_noCoprocessor, 0);
    installInterrupt(0x8, &expt_doubleFault, 0);
    installInterrupt(0xA, &expt_invalidTSS, 0);
    installInterrupt(0xB, &expt_segNotPresent, 0);
    installInterrupt(0xC, &expt_stackFault, 0);
    installInterrupt(0xD, &expt_generalProtection, 0);
    installInterrupt(0xE, &expt_pageFault, 0);
    installInterrupt(0x10, &expt_mathFault, 0);
    installInterrupt(0x11, &expt_alignCheck, 0);
    installInterrupt(0x12, &expt_machineCheck, 0);
    installInterrupt(0x13, &expt_simdFailure, 0);
    
    //And the syscall interface
    installInterrupt(0xFE, &expt_forceenter, 3);
    installInterrupt(0xFF, &expt_syscall, 3);
}
