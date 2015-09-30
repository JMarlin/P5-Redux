#include "../include/p5.h"
#include "../include/registrar.h"
#include "../include/pci.h"
#include "../include/usb.h"

void outb(unsigned short _port, unsigned char _data) {

    __asm__ __volatile__ ("out %%al, %%dx" : : "d" (_port), "a" (_data));
}


unsigned char inb(unsigned short _port) {

    unsigned char data;

    __asm__ __volatile__ ("in %%dx, %%al" : "=a" (data) : "dN" (_port));
    return data;
}


void outw(unsigned short _port, unsigned short _data) {

    __asm__ __volatile__ ("out %%ax, %%dx" : : "d" (_port), "a" (_data));
}


unsigned short inw(unsigned short _port) {


    unsigned short data;

    __asm__ __volatile__ ("in %%dx, %%ax" : "=a" (data) : "dN" (_port));
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

    unsigned int i, j;
	message temp_msg;
    unsigned char rev;
    unsigned int devcount;
	unsigned int parent_pid;
    unsigned short usb_base;
    unsigned int *usb_ram = (unsigned int*)getSharedPage();
    unsigned int address_test = 0;
    unsigned char inbuf[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

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
    prints("Do UHCI enumeration? [Y/N]: ");
    scans(9, inbuf);

    if(inbuf[0] == 'Y' || inbuf[0] == 'y') {

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
                prints("  USBSTS: 0x");
                printHexWord(inw(usb_base + 0x02));
                prints("\n[uhci]      USBINTR: 0x");
                printHexWord(inw(usb_base + 0x04));
                prints("  FRNUM: 0x");
                printHexWord(inw(usb_base + 0x06));
                //Set the base address
                outd(usb_base + 0x08, (unsigned int)usb_ram);
                //Wait for the base address to be set
                while(address_test != (unsigned int)usb_ram) {
                    address_test = ind(usb_base + 0x08) & 0xFFFFF000;
                    prints("\n[uhci]      FLBASEADD: 0x");
                    printHexDword(address_test);
                }
                prints("\n[uhci]      FLBASEADD: 0x");
                printHexDword(address_test);
                pchar('/');
                printHexDword((unsigned int)usb_ram);
                prints("  SOFMOD: 0x");
                printHexByte(inb(usb_base + 0x0C));
                prints("\n[uhci]      PORTSC1: 0x");
                printHexWord(inw(usb_base + 0x10));
                prints("  PORTSC2: 0x");
                printHexWord(inw(usb_base + 0x12));
                pchar('\n');

                //Allocate a physical page of memory at the host controller's default physical address
                allocatePhysical((void*)usb_ram, 0x1000);

                //Disable the controller and its ports
                prints("Disabling ports\n");
                outw(usb_base, inw(usb_base) & 0xFFFE); //Set run/stop to stop

                //Reset the host controller
                outw(usb_base, 0x0004); //Assert hcreset
                j = 0;
                while(j < 0xFFFFF) j++; //Wait 10ms (shitty timing)
                outw(usb_base, inw(usb_base) & 0x0002); //Clear greset, don't touch hcreset
                while((inw(usb_base) & 0x0002) == 0x0002); //Make sure that the controller isn't still resetting

                outw(usb_base + 0x10, 0x000A); //Disable port 1
                while(inw(usb_base + 0x10) & 0x0004); //Wait for the port to be disabled
                outw(usb_base + 0x12, 0x000A); //Disable port 2
                while(inw(usb_base + 0x12) & 0x0004); //Wait for the port to be disabled

                prints("Setting controller defaults\n");
                //Set up default controller state (no interrupts, debug on, )
                outw(usb_base + 0x02, inw(usb_base + 0x02) & 0x1F); //Clear USBSTS
                //Disable USB legacy support
                //pciWriteField((pci_address)i, pciReadField((pci_address)i, 0x30) & 0xFFFF0000 ); //We care about the low word
                outb(usb_base + 0x0C, 0x40); //Set SOF to default value, about 1ms per frame
                outw(usb_base + 0x04, 0x0); //Set all interrupts off
                outw(usb_base, 0x00E0); //Max packet = 64 (default), set configure flag, enable software debug, controller stopped

                //Make double sure the base address is set
                outd(usb_base + 0x08, (unsigned int)usb_ram);
                //Wait for the base address to be set
                while(address_test != (unsigned int)usb_ram) {
                    address_test = ind(usb_base + 0x08) & 0xFFFFF000;
                    prints("\n[uhci]      FLBASEADD: 0x");
                    printHexDword(address_test);
                }

                outw(usb_base + 0x06, 0x0); //Set the current frame number to 0

                //Enable port one, check to see if a device is installed
                prints("Enabling port 1 and checking for device\n");
                outw(usb_base + 0x10, 0x0004); //Enable port 1

                j = 0;
                while(j < 0xFFFFF && !(inw(usb_base + 0x10) & 0x0004))  //Wait for the port to be enabled
                    j++;

                if((j < 0xFFFFF) && (inw(usb_base + 0x10) & 0x0001)) {

                    //If device is installed, send a reset to the port
                    prints("Resetting device on port 1\n");
                    //Improve the timing later when we create a timer system
                    outw(usb_base + 0x10, 0x0204); //Port enabled, RESET state asserted
                    j = 0;               //Wait
                    while(j < 0xFFFFFFF)
                        j = j + 1;
                    outw(usb_base + 0x10, 0x0004); //Port enabled, RESET state cleared
                    while((inw(usb_base + 0x10) & 0x0200)); //Wait for reset to be lifted

                    //Send a resume just in case
                    prints("Resuming device on port 1\n");
                    //Improve the timing later when we create a timer system
                    outw(usb_base + 0x10, 0x0044); //Port enabled, RESUME state asserted
                    j = 0;               //Wait
                    while(j < 0xFFFFFFF)
                        j = j + 1;
                    outw(usb_base + 0x10, 0x0004); //Port enabled, RESUME state cleared
                    while((inw(usb_base + 0x10) & 0x0040)); //Wait for resume to be lifted

                    //We create a control transfer to device 0 control pipe:
                    //Create a SETUP address 0 packet TD with null next pointer, insert a reference to it into the frame list
                    prints("Setting up transfer structures\n");
                    usb_ram[0] = ((unsigned int)(&usb_ram[4])) | 0x00; //First frame list pointer, points to a td at usb_ram[16] (TDs are 16-byte aligned) and has 'terminate' un marked
                    usb_ram[4] = 0x1; //This starts the TD, and the first dword is the pointer to the next td. There is none, so this is marked with 'terminate'
                    usb_ram[5] = 0x1C800000; //Transfer active, Check to see later on if 0x800000 is set. If it's not, the transaction was carried out. And if 0x18000000 = 0x18000000 it had no errors
                    usb_ram[6] = 0x00E0002D; //eight bytes data, enpoint 0, address 0, PID 0x2D (SETUP)
                    usb_ram[7] = (unsigned int)&usb_ram[8]; //Pointer for the buffer that the controller should read the packet data from
                    usb_ram[8] = 0x01000680; //bmRequestType = 0x80 (device, standard, device-to-host), bRequest = 0x06 (descriptor), wValue = 0x0100 (device descriptor/descriptor index 0)
                    usb_ram[9] = 0x00120000; //wIndex = 0x0000 (unused in this request), wLength = 0x0012 (18 bytes, which is the length of a device descriptor)

                    //Set frame number to 0
                    outw(usb_base + 0x06, 0x0); //Set the current frame number to 0

                    //Set run/stop to run
                    //We should clear HCHalted here, to make sure we're not reading our own value later on
                    outw(usb_base + 0x02, 0x20); //The usbsts register is R/WC which means that writing a one to a location resets its value to zero
                    prints("Setting host controller to run...\n");
                    outw(usb_base, inw(usb_base) | 0x0001);
                    while(inw(usb_base + 0x02) & 0x20); //Wait until the halted status clears

                    //Poll HCHalted until the device is halted
                    while(!(inw(usb_base + 0x02) & 0x20));

                    //Display TD status
                    if(usb_ram[5] & 0x18800000)  //This is wrong logic. 0x800000 should be CLEARED if the transaction was processed
                        prints("Setup completed successfully");
                    else
                        prints("Setup failed");

                    //Display the status of the transaction descriptor
                    prints(" (");
                    printHexDword(usb_ram[5]);
                    prints(") USBSTS: 0x");
                    printHexWord(inw(usb_base + 0x02));
                    pchar('\n');

                    //Create a IN address 0 packet TD with null next pointer, insert a reference to it into the frame list
                    //Set frame number to 0
                    //Set run/stop to run
                    //Poll HCHalted until the device is halted
                    //Display TD status
                    //Display recieved data
                    //Create an OUT address 0 packet TD with null next pointer (data length 0, STATUS PHASE), insert a reference to it into the frame list
                    //Set frame number to 0
                    //Set run/stop to run
                    //Poll HCHalted until the device is halted
                    //Display TD status
                } else {

                    prints("No device found on port 1\n");
                }

                //Enable port one, check to see if a device is installed
                prints("Enabling port 2 and checking for device\n");
                outw(usb_base + 0x12, 0x0004); //Enable port 1

                j = 0;
                while(j < 0xFFFFF && !(inw(usb_base + 0x12) & 0x0004))  //Wait for the port to be enabled
                    j++;

                if((j < 0xFFFFF) && (inw(usb_base + 0x12) & 0x0001)) {

                    //If device is installed, send a reset to the port
                    prints("Resetting device on port 2\n");
                    //Improve the timing later when we create a timer system
                    outw(usb_base + 0x12, 0x0204); //Port enabled, RESET state asserted
                    j = 0;               //Wait
                    while(j < 0xFFFFFFF)
                        j = j + 1;
                    outw(usb_base + 0x12, 0x0004); //Port enabled, RESET state cleared
                    while((inw(usb_base + 0x12) & 0x0200)); //Wait for reset to be lifted

                    //We create a control transfer to device 0 control pipe:
                    //Create a SETUP address 0 packet TD with null next pointer, insert a reference to it into the frame list
                    prints("Setting up transfer structures\n");
                    usb_ram[0] = ((unsigned int)(&usb_ram[4])) | 0x00; //First frame list pointer, points to a td at usb_ram[16] (TDs are 16-byte aligned) and has 'terminate' un marked
                    usb_ram[4] = 0x1; //This starts the TD, and the first dword is the pointer to the next td. There is none, so this is marked with 'terminate'
                    usb_ram[5] = 0x1C800000; //Transfer active, Check to see later on if 0x800000 is set. If it's not, the transaction was carried out. And if 0x18000000 = 0x18000000 it had no errors
                    usb_ram[6] = 0x0100002D; //eight bytes data, enpoint 0, address 0, PID 0x2D (SETUP)
                    usb_ram[7] = (unsigned int)&usb_ram[8]; //Pointer for the buffer that the controller should read the packet data from
                    usb_ram[8] = 0x01000680; //bmRequestType = 0x80 (device, standard, device-to-host), bRequest = 0x06 (descriptor), wValue = 0x0100 (device descriptor/descriptor index 0)
                    usb_ram[9] = 0x00120000; //wIndex = 0x0000 (unused in this request), wLength = 0x0012 (18 bytes, which is the length of a device descriptor)

                    //Set frame number to 0
                    outw(usb_base + 0x06, 0x0); //Set the current frame number to 0

                    while(1) {

                        //Set run/stop to run
                        //We should clear HCHalted here, to make sure we're not reading our own value later on
                        outw(usb_base + 0x02, 0x20); //The usbsts register is R/WC which means that writing a one to a location resets its value to zero
                        prints("Setting host controller to run...\n");
                        outw(usb_base, inw(usb_base) | 0x0001);
                        while(inw(usb_base + 0x02) & 0x20); //Wait until the halted status clears

                        //Poll HCHalted until the device is halted
                        while(!(inw(usb_base + 0x02) & 0x20));

                        //Display the status of the transaction descriptor
                        prints(" (");
                        printHexDword(usb_ram[5]);
                        prints(") USBSTS: 0x");
                        printHexWord(inw(usb_base + 0x02));
                        pchar('\n');

                        //Display TD status
                        if(!(usb_ram[5] & 0x800000)) {

                            prints("Setup completed successfully\n");
                            break;
                        }

                        if(!(usb_ram[5] & 0x18000000)) {

                            prints("Could not recover from transfer errors.\n");
                            break;
                        }

                        //Create a IN address 0 packet TD with null next pointer, insert a reference to it into the frame list
                        //Set frame number to 0
                        //Set run/stop to run
                        //Poll HCHalted until the device is halted
                        //Display TD status
                        //Display recieved data
                        //Create an OUT address 0 packet TD with null next pointer (data length 0, STATUS PHASE), insert a reference to it into the frame list
                        //Set frame number to 0
                        //Set run/stop to run
                        //Poll HCHalted until the device is halted
                        //Display TD status
                    }
                } else {

                    prints("No device found on port 2\n");
                }

                while(1); //Temporarily hang once we're done with the first port for testing
            }
        }
    }

	prints("[uhci] Done.\n");
    postMessage(parent_pid, 0, 1); //Tell the parent we're done registering

	//Now that everything is set up, we can loop waiting for requests
	while(1) {

		getMessage(&temp_msg);

		switch(temp_msg.command) {

			default:
			break;
		}
	}
}
