#ifndef INT_H
#define INT_H

#define IDT_TYPE_INT32 0xE

typedef void (intHandler*)(void);

typedef struct idtPtr {
        unsigned char limit;
        unsigned int base;
} idtPtr;

typedef struct idtEntry {
        unsigned short offset_1;
        unsigned short selector;
        unsigned char zero;
        unsigned char type_attr;
        unsigned short offset_2;
} idtEntry;

#endif //INT_H
