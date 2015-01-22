#include "int.h" 
#include "../ascii_io/ascii_o.h"

unsigned char* idtBase = (idtPtr*)0x201000;
unsigned char* idtEntries = (idtEntry*)0x201030; //ends 0x201830  

void genericInterrupt(void) {
        __asm__ ("cli");
        prints("UNHANDLED INTERRUPT");
//        __asm__ ("iret");
        for(;;);
}

void set_idt(char* ptr) {
        prints("Setting idt to "); printHexDword((unsigned int)ptr); prints("\n");
        __asm__ (
            "lidt (%0)\n"
            :  
            : "p" (ptr)
        );
}

void installInterrupt(unsigned char number, intHandler handler) {

        unsigned char* entryBase = ((unsigned char*)idtEntries)+(8*number);

        unsigned short* offset_1 = (unsigned short*)entryBase;
        unsigned short* selector = (unsigned short*)(entryBase+2);
        unsigned char* zero = (unsigned char*)(entryBase+4);
        unsigned char* type_attr = (unsigned char*)(entryBase+5); 
        unsigned short* offset_2 = (unsigned short*)(entryBase+6);

        offset_1[0] = (unsigned short)(((unsigned int)handler) & 0xFFFF);
        selector[0] = 0x8; //The global code segment set up by the bootloader
        zero[0] = 0x0;
        type_attr[0] = 0x8E; //0x80 & IDT_TYPE_INT32; //0x00 = not present, minimum priviledge ring 00b, storage segment 0 
        offset_2[0] = (unsigned short)(((unsigned int)handler) >> 16);

}

void printIdt(unsigned char number) {

        unsigned char* entryBase = ((unsigned char*)idtEntries)+(8*number);

        unsigned short* offset_1 = (unsigned short*)entryBase;
        unsigned short* selector = (unsigned short*)(entryBase+2);
        unsigned char* zero = (unsigned char*)(entryBase+4);
        unsigned char* type_attr = (unsigned char*)(entryBase+5); 
        unsigned short* offset_2 = (unsigned short*)(entryBase+6);
        
        prints("\nHandler #"); printHexByte(number); prints(" @ "); printHexDword(entryBase);
        prints("\n   offset_1: "); printHexWord(offset_1[0]);
        prints("\n   selector: "); printHexWord(selector[0]);
        prints("\n   zero: "); printHexByte(zero[0]);
        prints("\n   type_attr: "); printHexByte(type_attr[0]); 
        prints("\n   offset_2: "); printHexWord(offset_2[0]);

}

void blankInterrupt(unsigned char number) {
        installInterrupt(number, &genericInterrupt);
}

void initIDT() {

        int i;       
        unsigned short* count = (unsigned short*)idtBase;
        unsigned int* ptr = (unsigned int*)(idtBase + 2); 

        prints("\nSetting idtBase.limit\n");
        count[0] = 0x800;
        prints("Setting idtBase.base\n");
        ptr[0] = (unsigned int)idtEntries;

        prints("Blanking interrupts\n");
        for(i = 0; i < 256; i++) {
                blankInterrupt(i);
        }                                                 

        printIdt(0);

        prints("Installing idt\n");
        set_idt(idtBase);
        //__asm__ ("lidt $0x201000");

}

