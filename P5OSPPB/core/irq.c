#include "irq.h"
#include "int.h"
#include "util.h"
#include "../ascii_io/ascii_o.h"
#include "../process/process.h"
#include "../process/message.h"
#include "../kserver/kserver.h"
#include "../core/global.h"

//Tables which will keep track of IRQ registration
process* irq_process[15] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

typedef void (*handler)(void);

extern void irq_handler_1(void);
extern void irq_handler_2(void);
extern void irq_handler_3(void);
extern void irq_handler_4(void);
extern void irq_handler_5(void);
extern void irq_handler_6(void);
extern void irq_handler_7(void);
extern void irq_handler_8(void);
extern void irq_handler_9(void);
extern void irq_handler_10(void);
extern void irq_handler_11(void);
extern void irq_handler_12(void);
extern void irq_handler_13(void);
extern void irq_handler_14(void);
extern void irq_handler_15(void);

handler irq_handler[15] = {
    &irq_handler_1,
    &irq_handler_2,
    &irq_handler_3,
    &irq_handler_4,
    &irq_handler_5,
    &irq_handler_6,
    &irq_handler_7,
    &irq_handler_8,
    &irq_handler_9,
    &irq_handler_10,
    &irq_handler_11,
    &irq_handler_12,
    &irq_handler_13,
    &irq_handler_14,
    &irq_handler_15
};

int irq_working[15] = {0};

void send_pic_eoi(unsigned char irq) {
	
	if(irq > 7)
		outb(PIC2_COMMAND, 0x20);

	outb(PIC1_COMMAND, 0x20);
}

//Open an IRQ channel on the PIC and set up the IRQ handler to pass a message to
//the registered process
//NOTE: irq_number is offset by one (eg: irq_number 0 implies irq 1) because
//irq 0 itself is reserved for the system timer regardless.
unsigned int irq_register(unsigned int irq_number, process *requesting_proc) {

    //Return failure if a nonsense IRQ was requested
    if(irq_number > 15)
        return 0;

    //map the process
    DEBUG("\nRegistered IRQ slot #");
    DEBUG_HD(irq_number);
    DEBUG("/interrupt 0x");
    DEBUG_HD(irq_number + 0xE0);
    DEBUG_PCHAR('\n');
    irq_process[irq_number - 1] = requesting_proc;

    //map the handler into the right interrupt vector, assuming it's not
    //already set up (E0 is irq 0)
    installInterrupt(irq_number + 0xE0, irq_handler[irq_number - 1], 3);

    //Open up the associated PIC channel
    enable_irq(irq_number);

    //For now, we can't really fail so we just return OK
    return 1;
}

unsigned int irq_deregister(unsigned int irq_number) {

    //Return failure if a nonsense IRQ was requested
    if(irq_number > 15)
        return 0;

    //unmap the process
    irq_process[irq_number - 1] = 0;

    //Remove the interrupt handler
    blankInterrupt(irq_number + 0xE0);

    //Close the associated PIC channel
    disable_irq(irq_number);

    //For now, we can't really fail so we just return OK
    return 1;
}

//This is the magic sauce which forces a message back to the registered proc
process* irq_handle(unsigned char irq_number) {

    DEBUG("\nIRQ ");
    DEBUG_HD(irq_number);
    DEBUG(" triggered\n");

    irq_number -= 0xE0;

    //Get the heck out of here if the irq isn't registered
    if(!irq_process[irq_number - 1]) {
        
        DEBUG("Nothing registered for this IRQ");   
        return (process*)0;
    }

    //Otherwise, write a message to the handling process and enter it
    passMessage(0, irq_process[irq_number - 1]->id, KS_REG_IRQ_1 + irq_number - 1, 0);
    
    //Tell the PIC we're ready to accept further interrupts on this channel
    send_pic_eoi(irq_number);

    //Unregister the process so that the client won't get another interrupt until it asks for it
    process* ret_proc = irq_process[irq_number - 1];
    irq_deregister(irq_number);   

    return ret_proc;
}

void enable_irq(unsigned char channel) {

        //The below was edited but not tested. The addition made was that 
        //IRQ2 is enabled as well for all IRQs above 7 as IRQ2 is how the
        //slave PIC communicates with the master PIC.

	if(channel > 7) {
            
            outb(PIC2_DATA, inb(PIC2_DATA) & ~(1 << (channel - 8)));
            enable_irq(2);
	} else { 
	    
	    outb(PIC1_DATA, inb(PIC1_DATA) & ~(1 << channel));
	}
}

void disable_irq(unsigned char channel) {
	
	if(channel > 7)
    	outb(PIC2_DATA, inb(PIC2_DATA) | (1 << (channel - 8)));
	else	
		outb(PIC1_DATA, inb(PIC1_DATA) | (1 << channel));
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
