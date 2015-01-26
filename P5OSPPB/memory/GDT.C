#include "gdt.h"


//'seg' is the actual segment number
void setGdtEntry(unsigned char seg, unsigned int limit, unsigned int base, unsigned char access, unsigned char flags) {
    
    gdt_entry* entry = (gdt_entry*)(GDT_START + seg); 
    entry->limit_low = (unsigned short)(limit & 0xFFFF);
    entry->flags_and_limit = (unsigned char)(((flags & 0xF) << 4) | ((limit & 0xF0000) >> 16));
    entry->base_low = (unsigned short)(base & 0xFFFF);
    entry->base_mid = (unsigned char)((base & 0xFF0000) >> 16);
    entry->base_hi = (unsigned char)((base & 0xFF000000) >> 24);
    entry->access = access; 
}


void lgdt(char* gdtr_ptr) {

    __asm__ (
        "lgdt (%0)\n"
        :  
        : "p" (gdtr_ptr)
    );
}


void installGdt(unsigned int table_ptr, unsigned short table_sz) {
    
    gdtr_val* gdtr = (gdtr_val*)GDTR_LOC;
    gdtr->limit = table_sz;
    gdtr->pointer = table_ptr;
    lgdt((char*)GDTR_LOC);          
}

void initGdt(void) {
    
    //Null
    setGdtEntry(0x00, 0, 0, 0, 0);
    
    //Kernel code, kernel data
    setGdtEntry(0x08, 0xFFFFF, 0x0, 0x9A, 0xC);
    setGdtEntry(0x10, 0xFFFFF, 0x0, 0x92, 0xC);
    
    //User code, user data
    setGdtEntry(0x18, 0xFFFFF, 0x0, 0xFA, 0xC);
    setGdtEntry(0x20, 0xFFFFF, 0x0, 0xF2, 0xC);  
    installGdt((unsigned int)GDT_START, (unsigned short)((5 * sizeof(gdt_entry)) - 1));

    //we'll shove the TSS in here in the future.
}
