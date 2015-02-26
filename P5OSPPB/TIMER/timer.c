#include "../core/util.h"
#include "../core/int.h"
#include "../process/process.h"
#include "timer.h"


void timer_on() {
    
    //Clear IRQ 0 mask
    outb(PIC1_DATA, inb(PIC1_DATA) & 0xFE);
}


void timer_off() {
    
    //Mask IRQ 0
    outb(PIC1_DATA, inb(PIC1_DATA) | 0x01);
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


//If and when we open up IRQ7 this will have to 
//be updated to actually check the PIC
void c_spurious_handler() {

    prints("SPURIOUS INTERRUPT\n");
    timer_int_ack();
}


void c_timer_handler() {

    t_counter++;    
        
    if(t_counter >= 1000) {
        
        t_counter = 0;
        
        //If we're in userland, force a task switch
        //otherwise, just set the next process to be swapped in 
        //when the kernel service is done
        if(in_kernel) {
            
            timer_int_ack();
            prep_next_process();
        } else {
            
            timer_int_ack();
            next_process();
        }    
        
    } else {
    
        timer_int_ack();
    }
}


void init_pic() {
    
    //Some of this IO may need delays to wait for the PIC
    //to finish, so this is a place to look at in the 
    //future if this ends up not running on real hardware
    outb(PIC1_COMMAND, 0x11); //0x10 init mode | 0x01 there will be four commands
    outb(PIC2_COMMAND, 0x11);
    outb(PIC1_DATA, 0xE0); //0xE0 will be the first IRQ vector (eg: IRQ0 -> int 0xE0)
    outb(PIC2_DATA, 0xE8); //0xE8 will be the ninth IRQ vector (eg: IRQ8 -> int 0xE8)
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

     unsigned short reload;

    if(freq <= 18) {
        reload = 0xFFFF;
    } else if(freq >= 596591) {
        reload = 0x0002;
    } else {
        reload = (unsigned short)((1193182 / freq) & 0xFFFF);
    }
    
    //Channel 0 mode 2
    outb(TIMER_COMMAND, 0x34);
    
    prints("Setting up timer with divisor 0x"); printHexWord(reload); prints("\n");
    
    //Load reload count
    outb(TIMER0_DATA, reload & 0xFF);
    outb(TIMER0_DATA, reload >> 8);
}


void init_timer() {

    t_counter = 0;

    init_pic();
    init_time_chip(1000);
    installInterrupt(TIMER_INT_NUM, &timer_handler, 0);
    //installInterrupt(0xE7, &spurious_handler, 0);
}
