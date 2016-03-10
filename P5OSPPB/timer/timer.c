#include "../core/util.h"
#include "../core/int.h"
#include "../core/global.h"
#include "../core/irq.h"
#include "../process/process.h"
#include "../process/message.h"
#include "../kserver/kserver.h"
#include "timer.h"

extern unsigned char _in_kernel;
extern unsigned char needs_swap;
extern unsigned char _was_spurious;

typedef struct timer_entry {
    process* p;
    unsigned int count;
    unsigned int limit;
} timer_entry;

unsigned int active_timer_count = 0;
timer_entry event_timer[10];

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

void timer_int_ack() {

    send_pic_eoi(0);
}

unsigned int install_timer_entry(process* target_p, unsigned int limit) {

    int i;

    //Fail if we're already full up on counters
    if(active_timer_count == 10)
        return 0;

    //Otherwise, find an open slot and install
    for(i = 0; i < 10; i++) {

        if(!(event_timer[i].p)) {

            event_timer[i].p = target_p;
            event_timer[i].count = 0;
            event_timer[i].limit = limit;
            active_timer_count++;

            return 1;
        }
    }

    //Odd case that our timer count is out of sync with our table for
    //some inconceivable reason
    return 0;
}

//Increment all active timer counters by one (this is fired on the millisecond)
//While we're at it, see if any of the timers have surpassed their limit and
//return the first one that has.
process* find_elapsed_timers() {

    int i, found = 0;
    process* ret_p = (process*)0;

    if(!active_timer_count)
        return (process*)0;

    //Look through the timer table and message and return the process of the first
    //found to exist and have elapsed
    for(i = 0; i < 10; i++) {

        if(event_timer[i].p) {

            event_timer[i].count++;

            if(event_timer[i].count >= event_timer[i].limit) {

                //Keep track of the matched process
                if(!ret_p) {
                    ret_p = event_timer[i].p;

                    //Uninstall the timer now that it's been exhausted
                    active_timer_count--;
                    event_timer[i].p = (process*)0;
                    event_timer[i].count = 0;
                    event_timer[i].limit = 0;

                    //Send the process a timer elapsed message
                    passMessage(0, ret_p->id, KS_TIMER, 1);
                }
            }
        }
    }

    return ret_p;
}

process* c_timer_handler() {

    static unsigned int tick_count = 0;
    process* timer_proc = (process*)0;

    //Check on our process switch regulation
    if(++tick_count >= TICK_THRESHOLD) {

        tick_count = 0;
        needs_swap = 1;
    }

    //Check to see if there are any timers that need handling
    timer_proc = find_elapsed_timers();

    //Tell the PIC that we're done handling the interrupt
    timer_int_ack();

    //Don't force process switch if we found an elapsed timer
    if(timer_proc)
        needs_swap = 0;

    return timer_proc;
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

    installInterrupt(TIMER_INT_NUM, &_calc_mips, 3);
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
        resetProcessCounter();
        installInterrupt(TIMER_INT_NUM, &_handle_timerInt, 3); //Reset the vector to the normal timer handler
        (*reentry_call)(_mips_counter);
    }
}

void init_timer() {

    t_counter = 0;
    active_timer_count = 0;
    int i;

    //Clear the event timer table
    for(i = 0; i < 10; i++)
        event_timer[i].p = (process*)0;

    init_pic();
    init_time_chip(1000); //We use this initial low speed for MIPS calc //UPDATE: As of now, we're just doing it because we want a steady ~1ms timebase for timer functions
    installInterrupt(TIMER_INT_NUM, &_handle_timerInt, 3);
    installInterrupt(0xE7, &_spurious_handler, 3);
}
