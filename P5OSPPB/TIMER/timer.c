#include "../core/util.h"
#include "../core/int.h"
#include "timer.h"


void timer_on() {
    
    //Clear IRQ 0 mask
    outb(PIC1_DATA, inb(PIC1_DATA) & 0x7F);
}


void timer_off() {
    
    //Mask IRQ 0
    outb(PIC1_DATA, inb(PIC1_DATA) | 0x80);
}


void send_pic_eoi(unsigned char irq)
{
	if(irq > 7)
		outb(PIC2_COMMAND, 0x20);
 
	outb(PIC1_COMMAND, 0x20);
}


void timer_int_ack() {

    send_pic_eoi(0);
}


void init_pic() {
    
    //Some of this IO may need delays to wait for the PIC
    //to finish, so this is a place to look at in the 
    //future if this ends up not running on real hardware
    outb(PIC1_COMMAND, 0x11); //0x10 init mode | 0x01 there will be four commands
    outb(PIC2_COMMAND, 0x11);
    outb(PIC1_DATA, 0xEF); //0xEF will be the first IRQ vector (eg: IRQ0 -> int 0xEF)
    outb(PIC2_DATA, 0xF7); //0xF7 will be the ninth IRQ vector (eg: IRQ8 -> int 0xF7)
    outb(PIC1_DATA, 0x04); //Identify slave exists at IRQ2
    outb(PIC2_DATA, 0x02); //Identify that PIC is a slave
    outb(PIC1_DATA, 0x01); //Set to 8086 mode
    outb(PIC2_DATA, 0x01);
    
    //For now, we're just going to mask everything but the timer
    //some time in the future, we'll spin off the PIC lib
    //to do fancy DMA stuff, but we're saying fuck it for now
    //Actually, mask the timer as well so it doesn't start right
    //off the bat
    outb(PIC1_DATA, 0xFF);
    outb(PIC2_DATA, 0xFF);
}


void init_time_chip(unsigned int freq) {

    unsigned short reload = (unsigned short)((1193182/ freq) & 0xFFFF);
    
    //Channel 0 mode 2
    outb(TIMER_COMMAND, 0x34);
    
    //Load reload count
    outb(TIMER0_DATA, (unsigned char)(reload & 0xFF));
    outb(TIMER0_DATA, (unsigned char)((reload & 0xFF00) >> 8));
}


void init_timer() {

    init_pic();
    init_time_chip(1000);
    installInterrupt(TIMER_INT_NUM, &timer_handler, 0);
}
