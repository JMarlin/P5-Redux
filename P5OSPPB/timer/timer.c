#include "../core/util.h"
#include "../core/int.h"
#include "../core/global.h"
#include "../core/irq.h"
#include "../process/process.h"
#include "timer.h"

extern unsigned char _in_kernel;
extern unsigned char needs_swap;
extern unsigned char _was_spurious;

void timer_on() {

    //Clear IRQ 0 mask
    enable_irq(0);
    //outb(PIC1_DATA, inb(PIC1_DATA) & 0xFE);
}


void timer_off() {

    //Mask IRQ 0
    disable_irq(0);
    //outb(PIC1_DATA, inb(PIC1_DATA) | 0x01);
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


void c_timer_handler() {

    static unsigned int tick_count = 0;

    if(++tick_count >= TICK_THRESHOLD) {

        //Force kernel entry
        tick_count = 0;
        needs_swap = 1;
	//pchar('.'); //For making sure the timer is actually firing
    }

    timer_int_ack();
}

//If and when we open up IRQ7 this will have to
//be updated to actually check the PIC
void c_spurious_handler() {

    DEBUG("SPURIOUS INTERRUPT\n");
    timer_int_ack();
    _was_spurious = 0;
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

    //prints("Setting up timer with divisor 0x"); printHexWord(reload); prints("\n");

    //Load reload count
    outb(TIMER0_DATA, reload & 0xFF);
    outb(TIMER0_DATA, reload >> 8);
}

//Stop the timer, update the frequency, and turn it back on
void throttle_timer(unsigned int freq) {

    timer_off();
    init_time_chip(freq); //Full throttle is 596591
    timer_on();
    installInterrupt(TIMER_INT_NUM, &_handle_timerInt, 3); //Make sure the normal handler is installed
}

void (*reentry_call)(unsigned int);
unsigned int state;
void do_mips_calc(void (*cb)(unsigned int)) {

    reentry_call = cb;
    state = 1;
    __asm__ ("jmp _mips_loop\n");
}

void c_calc_mips() {

    static unsigned int state = 1;

    //Acknowledge the timer interrupt
    timer_int_ack();

    if(state) {

        //We're starting up, so update the state and
        //re-enter the MIPS loop
        state = 0;
        __asm__ ("jmp _mips_loop\n");
    } else {

        //Do MIPS calculation from counter and return to
        //kernel startup process
        _mips_counter = _mips_counter * 400; //4 instructions in the loop and 100Hz sample window
        (*reentry_call)(_mips_counter);
    }
}

void init_timer() {

    t_counter = 0;

    init_pic();
    init_time_chip(100); //We use this initial low speed for MIPS calc
    installInterrupt(TIMER_INT_NUM, &_calc_mips, 3); //Initially, this interrupt will be trapped by the MIPS calculator
    installInterrupt(0xE7, &_spurious_handler, 3);
}
