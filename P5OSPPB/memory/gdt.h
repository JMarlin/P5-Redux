#ifndef GDT_H
#define GDT_H

#define GDTR_LOC 0x00201810
#define GDT_START 0x00201820
#define TSS_LOC 0x002019A0

typedef struct gdtr_val {
    unsigned short limit;
    unsigned int pointer;
} __attribute__ ((packed)) gdtr_val;

typedef struct gdt_entry {
    unsigned short limit_low;
    unsigned short base_low;
    unsigned char base_mid;
    unsigned char access;
    unsigned char flags_and_limit;
    unsigned char base_hi;
} __attribute__ ((packed)) gdt_entry;

typedef struct tss_entry {
   unsigned int prev_tss;   
   unsigned int esp0;       
   unsigned int ss0;        
   unsigned int esp1;        
   unsigned int ss1;
   unsigned int esp2;
   unsigned int ss2;
   unsigned int cr3;
   unsigned int eip;
   unsigned int eflags;
   unsigned int eax;
   unsigned int ecx;
   unsigned int edx;
   unsigned int ebx;
   unsigned int esp;
   unsigned int ebp;
   unsigned int esi;
   unsigned int edi;
   unsigned int es;         
   unsigned int cs;        
   unsigned int ss;        
   unsigned int ds;        
   unsigned int fs;       
   unsigned int gs;         
   unsigned int ldt;      
   unsigned short trap;
   unsigned short iomap_base;
} __attribute__ ((packed)) tss_entry;

void setGdtEntry(unsigned char num, unsigned int base, unsigned int limit, unsigned char access, unsigned char flags);
void installGdt(unsigned int table_ptr, unsigned short table_sz);
void initGdt(void);

#endif //GDT_H
