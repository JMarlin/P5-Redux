#ifndef INT_H
#define INT_H

#define IDT_TYPE_INT32 0xE

typedef void (*intHandler)(void);

typedef struct idtPtr {
    unsigned short limit;
    unsigned int base;
} __attribute__ ((packed)) idtPtr;

typedef struct idtEntry {
    unsigned short offset_1;
    unsigned short selector;
    unsigned char zero;
    unsigned char type_attr;
    unsigned short offset_2;
} __attribute__ ((packed)) idtEntry;

void blankInterrupt(unsigned char number);
void initIDT();
void installInterrupt(unsigned char number, intHandler handler);

#endif //INT_H
