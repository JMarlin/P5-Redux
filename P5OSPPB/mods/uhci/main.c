#include "../include/p5.h"
#include "../include/registrar.h"
#include "../include/pci.h"
#include "../include/usb.h"

void outb(unsigned short _port, unsigned char _data) {

    asm volatile ( "outb %0, %1" : : "a"(_data), "Nd"(_port) );
}


unsigned char inb(unsigned short _port) {

    unsigned char data;

    asm volatile ( "inb %1, %0" : "=a"(data) : "Nd"(_port) );
    return data;
}


void outw(unsigned short _port, unsigned short _data) {

    asm volatile (
        "push %%ax \n"
        "push %%dx \n"
        "mov %1, %%ax \n"
        "mov %0, %%dx \n"
        "out %%ax, %%dx \n"
        "pop %%dx \n"
        "pop %%ax \n"
        :
        : "r" (_port), "r" (_data)
        :
    );
}


unsigned short inw(unsigned short _port) {


    unsigned short data;

        asm volatile (
        "push %%ax \n"
        "push %%dx \n"
        "mov %1, %%dx \n"
        "in %%dx, %%ax \n"
        "mov %%ax, %0 \n"
        "pop %%dx \n"
        "pop %%ax \n"
        : "=r" (data)
        : "r" (_port)
        :
    );
    return data;
}

void outd(unsigned short _port, unsigned int _data) {

     __asm__ __volatile__ ("outl %%eax, %%dx" : : "d" (_port), "a" (_data));
}


 unsigned int ind(unsigned short _port) {


    unsigned int data;

    __asm__ __volatile__ ("inl %%dx, %%eax" : "=a" (data) : "dN" (_port));
    return data;
}

void main(void) {

    unsigned int i;
	message temp_msg;
    unsigned char rev;
    unsigned int devcount;
	unsigned int parent_pid;
    unsigned short usb_base;

	//Get the 'here's my pid' message from init
    getMessage(&temp_msg);
    parent_pid = temp_msg.source;
	prints("[uhci] Starting UHCI server...");

	//Register ourself with the registrar
	postMessage(REGISTRAR_PID, REG_REGISTER, SVC_USB);
	getMessage(&temp_msg);

	if(!temp_msg.payload) {

        prints("Failed.\n");
        postMessage(REGISTRAR_PID, REG_DEREGISTER, SVC_USB);
        postMessage(parent_pid, 0, 0); //Tell the parent we're done registering
        terminate();
    }

    //Init the PCI interface and scan for UHCI devices
    if(!pciInit()) {

        prints("Could not open PCI subsystem.\n");
        postMessage(REGISTRAR_PID, REG_DEREGISTER, SVC_USB);
        postMessage(parent_pid, 0, 0); //Tell the parent we're done registering
        terminate();
    }

    prints("scanning\n");
    devcount = pciDeviceCount();

    for(i = 0; i < devcount; i++) {

        if((pciGetDeviceClass((pci_address)i) & 0xFFFFFF00) == 0x0C030000) {

            //Print device banner
            prints("[uhci]    Found UHCI device at PCI address ");
            pchar('(');
            printHexByte((unsigned char)pciGetBus((pci_address)i));
            pchar(',');
            printHexByte((unsigned char)pciGetDevice((pci_address)i));
            pchar(',');
            printHexByte((unsigned char)pciGetFunction((pci_address)i));
            pchar(')');
            pchar('\n');

            //Print PCI register values
            prints("[uhci]      I/O Base Address: 0x");
            usb_base = (unsigned short)(pciReadField((pci_address)i, 0x8) & 0xFFE0);
            printHexWord(usb_base);
            prints("\n[uhci]      USB Revision: ");
            rev = (unsigned char)((pciReadField((pci_address)i, 0x18) & 0xFF000000) >> 24);

            switch(rev) {

                case 0x00:
                    prints("Pre-1.0");
                break;

                case 0x10:
                    prints("1.0");
                break;

                default:
                    pchar('\'');
                    printHexByte(rev);
                    pchar('\'');
                break;
            }

            pchar('\n');

            //Print USB control register states
            prints("[uhci]      USBCMD: 0x");
            printHexWord(inw(usb_base));
            prints("\n[uhci]      USBSTS: 0x");
            printHexWord(inw(usb_base + 0x02));
            prints("\n[uhci]      USBINTR: 0x");
            printHexWord(inw(usb_base + 0x04));
            prints("\n[uhci]      FRNUM: 0x");
            printHexWord(inw(usb_base + 0x06));
            prints("\n[uhci]      FLBASEADD: 0x");
            printHexDword(ind(usb_base + 0x08));
            prints("\n[uhci]      SOFMOD: 0x");
            printHexByte(inb(usb_base + 0x0C));
            prints("\n[uhci]      PORTSC1: 0x");
            printHexWord(inw(usb_base + 0x10));
            prints("\n[uhci]      PORTSC2: 0x");
            printHexWord(inw(usb_base + 0x12));
            pchar('\n');
        }
    }

    while(1); //Hang for now

	postMessage(parent_pid, 0, 1); //Tell the parent we're done registering
	prints("[uhci] Done.\n");

	//Now that everything is set up, we can loop waiting for requests
	while(1) {

		getMessage(&temp_msg);

		switch(temp_msg.command) {

			default:
			break;
		}
	}
}
