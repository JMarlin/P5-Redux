#ifndef TIMER_H
#define TIMER_H
	
#define PIC1_COMMAND   0x20
#define PIC1_DATA	   0x21
#define PIC2_COMMAND   0xA0
#define PIC2_DATA	   0xA1
#define TIMER0_DATA    0x40
#define TIMER_COMMAND  0x43

void timer_on();
void timer_off();
void init_timer();
void timer_int_ack();

extern void timer_handler(void);

#endif //TIMER_H
