#ifndef IRQ_H
#define IRQ_H

#define PIC1_COMMAND   0x20
#define PIC1_DATA	   0x21
#define PIC2_COMMAND   0xA0
#define PIC2_DATA	   0xA1

#include "../process/process.h"

process* irq_handle(unsigned char irq_number);
void enable_irq(unsigned char channel);
void disable_irq(unsigned char channel);
void init_pic();
unsigned int irq_register(unsigned int irq_number, process *requesting_proc);

#endif //IRQ_H
