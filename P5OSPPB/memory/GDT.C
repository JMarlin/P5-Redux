#include "gdt.h"
#include "../ascii_io/ascii_o.h"

tss_entry* sys_tss = (tss_entry*)TSS_LOC;

void initTss(tss_entry* tgt_tss) {
    
    int i;
    unsigned char* byte_tss = (unsigned char*) tgt_tss;
    
    //Clear the TSS structure
    for(i = 0; i < sizeof(tss_entry); i++) {
        byte_tss[i] = 0;
    }
    
    //Need to hardcode the kernel stack location at some point
    tgt_tss->esp0 = 0x3FFFFF;
    tgt_tss->ss0 = 0x10;
}

//'seg' is the actual segment number
void setGdtEntry(unsigned char seg, unsigned int base, unsigned int limit, unsigned char access, unsigned char flags) {
    
    gdt_entry* entry = (gdt_entry*)(GDT_START + seg); 
    entry->limit_low = (unsigned short)(limit & 0xFFFF);
    entry->flags_and_limit = (unsigned char)(((flags & 0xF) << 4) | ((limit & 0xF0000) >> 16));
    entry->base_low = (unsigned short)(base & 0xFFFF);
    entry->base_mid = (unsigned char)((base & 0xFF0000) >> 16);
    entry->base_hi = (unsigned char)((base & 0xFF000000) >> 24);
    entry->access = access; 
}


void installTss(void) {
    
    __asm__ volatile (" mov $0x2B, %ax \n ltr %ax \n");
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
    setGdtEntry(0x08, 0x0, 0xFFFFF, 0x9A, 0xC);
    setGdtEntry(0x10, 0x0, 0xFFFFF, 0x92, 0xC);
    
    //User code, user data
    setGdtEntry(0x18, 0x0, 0xFFFFF, 0xFA, 0xC);
    setGdtEntry(0x20, 0x0, 0xFFFFF, 0xF2, 0xC);  
    
    //TSS 
    initTss(sys_tss);
    setGdtEntry(0x28, (unsigned int)sys_tss, 0x68, 0xE9, 0x8);
    installGdt((unsigned int)GDT_START, (unsigned short)((6 * sizeof(gdt_entry)) - 1));
    installTss();
}


//Enter V86 mode 
void jumpV86(unsigned int* entryPoint) {
    
    unsigned short entSeg = (unsigned short)((unsigned int)entryPoint >> 4);
    
    prints("entSeg = 0x");
    printHexWord(entSeg);
    prints("\n");

    __asm__ volatile (
        "mov $0x91000, %%esp \n"
        "mov $0x0, %%eax \n"
        "mov %0, %%ax \n"
        "push %%eax \n"
        "push %%eax \n"
        "push %%eax \n"
        "push %%eax \n"
        "mov $0x9000, %%ax \n"
        "push %%eax \n"
        "mov $0x1000, %%ax \n"
        "push %%eax \n"
        "pushf \n"
        "pop %%eax \n"
        "or $0x00020000, %%eax \n"
        "push %%eax \n"
        "mov $0x0, %%eax \n"
        "mov %0, %%ax \n"
        "push %%eax \n"
        "mov $0x0, %%eax \n"
        "push %%eax \n"
        "iret \n"
        : 
        : "r" (entSeg)
        : "eax"
    );
}

//Enter a function in user mode
void jumpUser(unsigned int* entryPoint) {

    __asm__ volatile (
        "mov $0x800FFF, %%esp \n"
        "mov $0x23, %%ax \n"
        "mov %%ax, %%ds \n"
        "mov %%ax, %%es \n"
        "mov %%ax, %%fs \n"
        "mov %%ax, %%gs \n"
        "mov %%esp, %%eax \n"
        "push $0x23 \n"
        "push %%eax \n"
        "pushf \n"
        "push $0x1B \n"
        "mov %0, %%eax \n"
        "push %%eax \n"
        "iret \n"
        : 
        : "r" (entryPoint)
        : "eax"
    );
}