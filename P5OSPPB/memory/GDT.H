#ifndef GDT_H
#define GDT_H

#define GDTR_LOC 0x00201810
#define GDT_START 0x00201820

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

void setGdtEntry(unsigned char num, unsigned int limit, unsigned int base, unsigned char access, unsigned char flags);
void installGdt(unsigned int table_ptr, unsigned short table_sz);
void initGdt(void);

#endif //GDT_H
