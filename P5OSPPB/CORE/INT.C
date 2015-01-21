#include "int.h" 
#include "../ascii_io/ascii_o.h"

idtPtr* idtBase = (idtPtr*)0x201000;
idtEntry* idtEntries = (idtEntry*)0x201030; //ends 0x201830  

void genericInterrupt(void) {
        __asm__ ("cli");
        prints("UNHANDLED INTERRUPT");
        while(1){}
}

void set_idt(idtPtr* ptr) {
        __asm__ __volatile__ ("lidt %0 \n" : : "m"(ptr));
}

void blankInterrupt(unsigned char number) {
//        asm ("cli");
        idtEntries[number].offset_1 = (unsigned short)(((unsigned int)&genericInterrupt) & 0xFFFF);
        idtEntries[number].selector = 0x8; //The global code segment set up by the bootloader
        idtEntries[number].zero = 0x0;
        idtEntries[number].type_attr = 0x80 & IDT_TYPE_INT32; //0x00 = not present, minimum priviledge ring 00b, storage segment 0 
        idtEntries[number].offset_2 = (unsigned short)(((unsigned int)&genericInterrupt) >> 16);
//        asm ("sti");
}

void initIDT() {

        int i;       

        prints("\nSetting idtBase.limit\n");
        idtBase[0].limit = 0x800;
        prints("Setting idtBase.base\n");
        idtBase[0].base = (unsigned int)idtEntries;

        prints("Blanking interrupts\n");
        for(i = 0; i < 256; i++) {
                blankInterrupt(i);
        }                                                 

        prints("Installing idt\n");
        set_idt(idtBase);

}

void installInterrupt(unsigned char number, intHandler handler) {
//        asm ("cli");
        idtEntries[number].offset_1 = (unsigned short)(((unsigned int)handler) & 0xFFFF);
        idtEntries[number].selector = 0x8; //The global code segment set up by the bootloader
        idtEntries[number].zero = 0x0;
        idtEntries[number].type_attr = 0x80 & IDT_TYPE_INT32; //0x80 = present, minimum priviledge ring 00b, storage segment 0 
        idtEntries[number].offset_2 = (unsigned short)(((unsigned int)handler) >> 16);
//        asm ("sti");
}
