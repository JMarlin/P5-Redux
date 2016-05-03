#include "../include/key.h"
#include "../include/p5.h"
#include "../include/registrar.h"

void outb(unsigned short _port, unsigned char _data) {

	asm volatile ( "outb %0, %1" : : "a"(_data), "Nd"(_port) );
}


unsigned char inb(unsigned short _port) {

	unsigned char data;

	asm volatile ( "inb %1, %0" : "=a"(data) : "Nd"(_port) );
	return data;
}

void uart_init() {

	outb(0x3f9, 0x00);
	outb(0x3fb, 0x80);
	outb(0x3f8, 0x03);
	outb(0x3f9, 0x00);
	outb(0x3fb, 0x03);
	outb(0x3fa, 0xc7);
	outb(0x3fc, 0x01);
}

int uart_hasData() {

	return inb(0x3fd) & 1;
}

unsigned char uart_getNextByte() {

    return inb(0x3f8);
}    

int rx_irq_regd = 0;

unsigned char read_index;
unsigned char write_index;
unsigned char buffer_full;
unsigned char buffer_empty;
unsigned char rx_buffer[256];

void initRxBuffer() {

    int i;

    for(i = 0; i < 256; i++)
        rx_buffer[i] = 0;

    read_index = 0;
    write_index = 0;
    buffer_full = 0;
    buffer_empty = 1;
}

unsigned char buffer_retrieve() {

    unsigned char c;

    if(read_index == write_index)
        return 0;

    c = rx_buffer[read_index++];
}

void buffer_insert(unsigned char c) {

    if((write_index == (read_index - 1)) || c == 0)
        return;

    rx_buffer[write_index++] = c;
}

//This thread will go to sleep waiting for the OS to wake it on a triggered IRQ 
//and then will interpret the codes found in the UART keybuffer into ascii data 
//and put it into the keyboard ring buffer
void rxIRQThread() {
	
	prints("[UART] Started interrupt thread\n");
	prints("[UART] Registering UART IRQ...");

	//Try to register the IRQ
	if(!registerIRQ(4)) {

	    prints("Failed.\n");
    	    terminate();
	}
	
	prints("Done.\n");
	
	rx_irq_regd = 1;
	
	while(1) {

	    waitForIRQ(4);    
        
            while(uart_hasData()) 
	        buffer_insert(uart_getNextByte());
	}
}

void keyMessageThread() {
    
    message temp_msg;
    unsigned char c;
    
    prints("[UART] Starting KEY service\n");
    
    //First thing, register as a KEY service with the registrar
    //We do this here so that we have this thread's PID instead of the parent's 
    postMessage(REGISTRAR_PID, REG_REGISTER, SVC_KEY);
    getMessage(&temp_msg);
    prints("[UART] KEY service registered.\n");
         
    //We should make sure the registration works, but for now we're just assuming it did
    rx_irq_regd = 2;    

    while(1) {
        
        getMessage(&temp_msg);
        
        if(temp_msg.command == KEY_GETCH) {
            
            //Wait until there's a character in the buffer 
            while(!(c = buffer_retrieve()));
            
            //Once we have one, post it back
            postMessage(temp_msg.source, KEY_GETCH, (unsigned int)c);
        }
    }
}

void main(void) {
	
    message temp_msg;
    unsigned int parent_pid;

    //Get the 'here's my pid' message from init
    getMessage(&temp_msg);
    parent_pid = temp_msg.source;

    //Enable interrupts on the UART controller
    prints("[UART] Enabling UART RX interrupts...");
    uart_init();
    initRxBuffer();
    prints("Done.\n");

    //Start the thread that will listen for the RX interrupts 
    if(!startThread())
        rxIRQThread();

    while(!rx_irq_regd);

    //Start the thread that will listen for keyboard client requests
    if(!startThread())
        keyMessageThread();

    while(rx_irq_regd != 2);

    postMessage(parent_pid, 0, 1); //Tell the parent we're done registering

    //With all of the threads started, the original core thread can exit
    terminate();
}
