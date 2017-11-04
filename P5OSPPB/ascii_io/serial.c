#include "../core/util.h"

unsigned short port;

void initSerial(unsigned short port_base) {

    port = port_base;

    //COM1 normal port base is 0x03F8
/*
    outb(0x3f9, 0x00);
    outb(0x3fb, 0x80);
    outb(0x3f8, 0x0C);
    outb(0x3f9, 0x00);
    outb(0x3fb, 0x03);
    outb(0x3fa, 0xc7);
    outb(0x3fc, 0x0b);
*/

    outb(port + 1, 0x00);
    outb(port + 3, 0x80);
    outb(port, 0x0C);
    outb(port + 1, 0x00);
    outb(port + 3, 0x03);
    outb(port + 2, 0xc7);
    outb(port + 4, 0x0b);
}


int txReady() {

    //return inb(0x3fd) & 0x20;
    return inb(port + 5) & 0x20;
}


void serPutch(unsigned char c) {

    while(!txReady());

    if(c == '\n') {

        //outb(0x3f8, 0xA);
        outb(port, 0xA);
        while(!txReady());
        //outb(0x3f8, 0xD);
        outb(port, 0xD);
    } else {
        
        //outb(0x3f8, c);
        outb(port, c);
    }
}

int serReceived() {
    
    return inb(port + 5) & 1;
}

char serGetch() {

    //return inb(0x3f8);
    return inb(port);
}
