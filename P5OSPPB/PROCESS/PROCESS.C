#include "../core/kernel.h"
#include "process.h"
#include "../ascii_io/ascii_o.h"


void kernelEntry(void) {

    prints("INTERRUPT HAS RETURNED CONTROL TO THE KERNEL\n");
    prints("Previous State:\n");
    prints("eax: 0x"); printHexDword(old_eax); prints("  ebx"); printHexDword(old_ebx); prints("\n");
    prints("ecx: 0x"); printHexDword(old_eax); prints("  edx"); printHexDword(old_ebx); prints("\n");
    prints("esp: 0x"); printHexDword(old_esp); prints("  ebp"); printHexDword(old_ebp); prints("\n");
    prints("esi: 0x"); printHexDword(old_esi); prints("  edi"); printHexDword(old_edi); prints("\n");
    prints("esp: 0x"); printHexDword(old_esp); prints("  cr3"); printHexDword(old_cr3); prints("\n");
    prints("eip: 0x"); printHexDword(old_eip); prints("  eflags"); printHexDword(old_eflags); prints("\n");
    prints("es: 0x"); printHexWord(old_es); prints("  cs"); printHexWord(old_cs); prints("\n");
    prints("ss: 0x"); printHexWord(old_ss); prints("  ds"); printHexWord(old_ds); prints("\n");
    prints("fs: 0x"); printHexWord(old_fs); prints("  gs"); printHexWord(old_gs); prints("\n");
}


void terminateProcess(proc* p) {

    //This is just some bullshit 'go back to console' placeholder
    sys_console();    
}
