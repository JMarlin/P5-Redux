#ifndef TIMER_H
#define TIMER_H
	
#define PIC1_COMMAND   0x20
#define PIC1_DATA	   0x21
#define PIC2_COMMAND   0xA0
#define PIC2_DATA	   0xA1
#define TIMER0_DATA    0x40
#define TIMER_COMMAND  0x43
#define TIMER_INT_NUM  0xE0

void timer_on();
void timer_off();
void init_timer();
void timer_int_ack();
void c_timer_handler();

extern void timer_handler(void);

#endif //TIMER_H
