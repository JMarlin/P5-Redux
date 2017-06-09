#include "../include/p5.h"
#include "../include/registrar.h"
#include "../include/blockdev.h"

//FDC Registers
#define STAT_REG_A         0x3F0 //RO
#define STAT_REG_B         0x3F1 //RO
#define DIGITAL_OUT_REG    0x3F2 //RW
#define TAPE_DRIVE_REG     0x3F3 //RW
#define MAIN_STATUS_REG    0x3F4 //RO
#define DATA_RATE_REG      0x3F4 //WO 
#define DATA_FIFO          0x3F5 //RW
#define DIGITAL_IN_REG     0x3F7 //RO 
#define CONFIG_CONTROL_REG 0x3F7 //WO

//DMA Registers
#define MSTR_START_0     0x00 //WO 16
#define MSTR_COUNT_0     0x01 //WO 16
#define MSTR_START_1     0x02 //WO 16
#define MSTR_COUNT_1     0x03 //WO 16
#define MSTR_START_2     0x04 //WO 16
#define MSTR_COUNT_2     0x05 //WO 16
#define MSTR_START_3     0x06 //WO 16
#define MSTR_COUNT_3     0x07 //WO 16
#define MSTR_STATUS      0x08 //RO 8
#define MSTR_COMMAND     0x08 //WO 8
#define MSTR_REQUEST     0x09 //WO 8
#define MSTR_SINGLE_MASK 0x0A //WO 8
#define MSTR_MODE        0x0B //WO 8
#define MSTR_FF_RESET    0x0C //WO 8
#define MSTR_IMMEDIATE   0x0D //RO 8
#define MSTR_RESET       0x0D //WO 8
#define MSTR_MASK_RESET  0x0E //WO 8
#define MSTR_MULTI_MASK  0x0F //WO 8
#define PAGE_ADDR_REG_0  0x87 //8
#define PAGE_ADDR_REG_1  0x83 //8
#define PAGE_ADDR_REG_2  0x81 //8
#define PAGE_ADDR_REG_3  0x82 //8
#define PAGE_ADDR_REG_4  0x8F //8
#define PAGE_ADDR_REG_5  0x8B //8
#define PAGE_ADDR_REG_6  0x89 //8
#define PAGE_ADDR_REG_7  0x8A //8


void outb(unsigned short _port, unsigned char _data) {

	asm volatile ( "outb %0, %1" : : "a"(_data), "Nd"(_port) );
}


unsigned char inb(unsigned short _port) {

	unsigned char data;

	asm volatile ( "inb %1, %0" : "=a"(data) : "Nd"(_port) );
	return data;
}

void outw(unsigned short _port, unsigned short _data) {

	asm volatile ( "outw %0, %1" : : "a"(_data), "Nd"(_port) );
}


unsigned short inw(unsigned short _port) {

	unsigned short data;

	asm volatile ( "inw %1, %0" : "=a"(data) : "Nd"(_port) );
	return data;
}

//DMA transfer procedure:
//   - Set up DMA channel #2 with byte_count -1, target address, direction
//      - ADD
//   - Reset/Init floppy controller as needed
//      - ADD 
//   - Select drive (we just start by assuming 1.44 meg A:)
//   - Set up FDC for DMA using specify command 
//   - Seek head to requested cylinder
//   - Issue sense interrupt command 
//   - Issue FDC R/W commands 
//   - Register for and wait for IRQ6
//   - Check 'result' FDC bytes for any errors

//NOTE: We'll start out not checking since P5 is written to pretend
//      the system only has 32M of RAM anyhow and we're not using
//      very much memory yet, but ISA DMA transfers can ONLY target
//      the first 16MB of RAM.

void main(void) {
	
	message temp_msg;
	unsigned int parent_pid;

	//Get the 'here's my pid' message from init
    getMessage(&temp_msg);
    parent_pid = temp_msg.source;

    //For now, we want to manually and in a very dirty way
    //set up the DMA for a transfer, set up the FDC,
    //fire off a read command, wait for the IRQ, check 
    //for error, and finally do a dump of the read 
    //data so that we can compare it to the floppy image
    //Once that works great, we'll clean things up a bit
    //and implement a client messaging interface for block
    //devices (steal the basic API concept from the kernel)
    //Finally, with that in place, we can move on to 
    //implementing a FAT12 filesystem provider service
    //and then a VFS server to sit on top of all of that
    //and provide high-level file IO services based on 
    //the currently loaded block device and filesystem
    //servers
    prints("[FDC] Beginning FDC service init...\n");
    prints("[FDC] Registering as a block device driver...");
    
    if(registerAsBlockDriver())
        prints("Success\n");
    else
        prints("Failed\n");

    //For now we're going to hang the system for testing
    //postMessage(parent_pid, 0, 1); //Tell the parent we're done registering

	while(1);
    //terminate();
}
