#include "../ascii_io/ascii_o.h"
#include "util.h"
#include "../core/global.h"


//Warning: This trashes memory at 0x1 and 0x100001.
//As such, it should only be used during init code.
int testA20() {

    //Choosing this randomly just because it should be in low free RAM just past the IDT loaded by PBOOT
    unsigned char* A20TestLow = (unsigned char*)0x1600;  
    unsigned char* A20TestHi = (unsigned char*)0x101600;
    unsigned char oldHi = A20TestHi[0];

    DEBUG("Setting low byte to 0\n");
    A20TestHi[0] = 0x00;
    DEBUG("Setting hi byte to FF\n");
    A20TestLow[0] = 0xFF;
    DEBUG("Testing the difference\n");
     
    if(A20TestHi[0] != 0xFF) {
        
        //Setting the lower value didn't affect the higher
        //value. A20 is already enabled, so quit true.
        return 1;
        A20TestHi[0] = oldHi;
    }

    A20TestHi[0] = oldHi;
    return 0;
}


int enableA20() {

    unsigned char KBCOutPortReg;
    int timeout;

    DEBUG("[enableA20()]: Attempting to enable the A20 line.\n");        

    //See if it's already enabled
    if(testA20()) {
        DEBUG("[enableA20()]: A20 is already enabled.\n");
        return 1;
    }        
    
    if(testA20()) {
    
        DEBUG("[enableA20()]: A20 enabled via BIOS call.\n");
        return 1;                   
    }

    DEBUG("Trying KBC method\n");
    
    //Try the keyboard controller

    //Wait for the KBC status reg bit 1  to clear (ready for input)
    while(inb(0x64)&0x2);

    //Disable keyboard
    outb(0x64, 0xAD);
    while(inb(0x64)&0x2);
    
    //Request read from Controller Output Port register
    outb(0x64, 0xD0);       
    
    //Wait for the KBC satus reg bit 0 to be set (data available)
    while(!(inb(0x64)&0x1));
    
    //Read the output port register value
    KBCOutPortReg = inb(0x60);    
    while(inb(0x64)&0x2);

    //Request write to KBC output port register
    outb(0x64, 0xD1);           
    while(inb(0x64)&0x2);
    
    //Write the value back with the A20 bit set
    outb(0x60, KBCOutPortReg|0x2);     
    while(inb(0x64)&0x2);
    
    //Re-enable the keyboard
    outb(0x64, 0xAE);       
    
    for(timeout = 0; timeout < 2000; timeout++) {
        
        if(testA20()) {
            DEBUG("[enableA20()]: A20 enabled via KBC.\n");
            return 1;
        }
    }

    DEBUG("Trying fast A20 method\n");
        
    //Try fast A20 method
    __asm__ volatile (
        "inb $0x92, %al\n\t"
        "or $0x2, %al\n\t"
        "out %al, $0x92"
    );
    
    if(testA20()) {
        prints("[enableA20()]: A20 enabled via Fast A20 port.");
        return 1;
    }

    prints("[enableA20()]: A20 could not be enabled.\n");
    return 0;    
}


inline void outb(unsigned short _port, unsigned char _data) {
    
    asm volatile ( "outb %0, %1" : : "a"(_data), "Nd"(_port) );
}


inline unsigned char inb(unsigned short _port) {
    
    unsigned char data;
    
    asm volatile ( "inb %1, %0" : "=a"(data) : "Nd"(_port) );
    return data;      
}


inline void outw(unsigned short _port, unsigned short _data) {

    asm volatile ( 
        "push %%ax \n"
        "push %%dx \n"
        "mov %1, %%ax \n"
        "mov %0, %%dx \n"
        "out %%ax, %%dx \n"
        "pop %%dx \n"
        "pop %%ax \n"
        : 
        : "r" (_port), "r" (_data)
        :
    );
}


inline unsigned short inw(unsigned short _port) {


    unsigned short data;
    
        asm volatile ( 
        "push %%ax \n"
        "push %%dx \n"
        "mov %1, %%dx \n"
        "in %%dx, %%ax \n"
        "mov %%ax, %0 \n"
        "pop %%dx \n"
        "pop %%ax \n"
        : "=r" (data)
        : "r" (_port)
        :
    );
    return data;
}


inline void outd(unsigned short _port, unsigned int _data) {

    asm volatile ( "outl %0, %1" : "=a"(_data) : "Nd"(_port) );
}


inline unsigned int ind(unsigned short _port) {


    unsigned int data;
    
    asm volatile ("inl %1, %0" : "=a"(data) : "Nd"(_port) );
    return data;
}
