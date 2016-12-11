#include "../core/util.h"


void initSerial() {

    outb(0x3f9, 0x00);
    outb(0x3fb, 0x80);
    outb(0x3f8, 0x0C);
    outb(0x3f9, 0x00);
    outb(0x3fb, 0x03);
    outb(0x3fa, 0xc7);
    outb(0x3fc, 0x0b);
}


int txReady() {

    return inb(0x3fd) & 0x20;
}


void serPutch(unsigned char c) {

    while(!txReady());

    if(c == '\n') {

        outb(0x3f8, 0xA);
        while(!txReady());
        outb(0x3f8, 0xD);
    } else {
        
        outb(0x3f8, c);
    }
}

int serReceived() {
    
    return inb(0x3f8 + 5) & 1;
}

char serGetch() {

    return inb(0x3f8);
}
