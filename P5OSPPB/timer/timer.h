#ifndef TIMER_H
#define TIMER_H

#define TIMER0_DATA    0x40
#define TIMER_COMMAND  0x43
#define TIMER_INT_NUM  0xE0
#define TICK_THRESHOLD 0

void timer_on();
void timer_off();
void init_timer();
void timer_int_ack();
void c_spurious_handler();
void c_timer_handler();
void c_calc_mips();
void irq_enter_kernel();
void do_mips_calc(void (*cb)(unsigned int));
void throttle_timer(unsigned int freq);

extern void _handle_timerInt(void);
extern void _spurious_handler(void);
extern void _calc_mips(void);
extern void _mips_loop(void);

extern unsigned int _mips_counter;

unsigned int t_counter;

#endif //TIMER_H
