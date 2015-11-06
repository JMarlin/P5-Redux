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
    unsigned int *frame_list = (unsigned int*)getSharedPage();
    unsigned char* usb_buf;
    unsigned int address_test = 0;
    unsigned char inbuf[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    unsigned char td1 = 0, td2 = 0;
    unsigned char port_count;
    unsigned short temp;
    unsigned short portreg, maxport;

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
                
                //Enable the device for memory and i/o access and bus master capability
                pciWriteField((pci_address)i, 1, pciReadField((pci_address)i, 1) | 0x7);

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
                outd(usb_base + 0x08, (unsigned int)frame_list);
                //Wait for the base address to be set
                while(address_test != (unsigned int)frame_list) {
                    address_test = ind(usb_base + 0x08) & 0xFFFFF000;
                    prints("\n[uhci]      FLBASEADD: 0x");
                    printHexDword(address_test);
                }
                prints("\n[uhci]      FLBASEADD: 0x");
                printHexDword(address_test);
                pchar('/');
                printHexDword((unsigned int)frame_list);
                prints("  SOFMOD: 0x");
                printHexByte(inb(usb_base + 0x0C));
                prints("\n[uhci]      PORTSC1: 0x");
                printHexWord(inw(usb_base + 0x10));
                prints("  PORTSC2: 0x");
                printHexWord(inw(usb_base + 0x12));
                pchar('\n');

                //Reset the host controller
                prints("Resetting the HC\n");
                outw(usb_base, inw(usb_base) | 0x0004); //Assert greset
                sleep(100); //Let the reset happen
                outw(usb_base, inw(usb_base) & ~((unsigned short)0x0004));
                for(j = 0; j < 20; j++)
                    if(!(inw(usb_base) & 0x0004))
                        break;                    
                if(inw(usb_base) & 0x0004)
                    prints("HC greset did not complete\n");
                outb(usb_base + 0x0C, 0x40); //Set SOF to default value, about 1ms per frame
                outw(usb_base + 0x04, 0x0); //Set all interrupts off
                outw(usb_base, 0x00E0); //Max packet = 64 (default), set configure flag, enable software debug, controller stopped
                //Set up default controller state (no interrupts, debug on, )
                //outw(usb_base + 0x02, inw(usb_base + 0x02) & 0x1F); //Clear USBSTS
                //Disable USB legacy support
                prints("Legsup: ");
                printHexDword(pciReadField((pci_address)i, 0x30));
                pciWriteField((pci_address)i, 0x30, (pciReadField((pci_address)i, 0x30) & 0xFFFF0000) | 0x8F00); //We care about the low word
                prints(" -> ");
                printHexDword(pciReadField((pci_address)i, 0x30));
                pchar('\n');

                //Detect and disable ports
                for(port_count = 0; port_count < 7; port_count++) {

                    temp = inw(usb_base + 0x10 + (port_count * 2));
                    if(!(temp & 0x80) || temp == 0xFFFF)
                        break;

                    outw(usb_base + 0x10 + (port_count * 2), 0x000A); //Disable port
                    while(inw(usb_base + 0x10 + (port_count * 2)) & 0x0004); //Wait for the port to be disabled
                }

                prints("Found ");
                printHexByte(port_count);
                prints(" host controller ports.\n");               
                
                //Make double sure the base address is set
                outd(usb_base + 0x08, (unsigned int)frame_list);
                //Wait for the base address to be set
                while(address_test != (unsigned int)frame_list) {
                    address_test = ind(usb_base + 0x08) & 0xFFFFF000;
                    prints("\n[uhci]      FLBASEADD: 0x");
                    printHexDword(address_test);
                }

                maxport = usb_base + 0x10 + (port_count * 2);

                for(portreg = usb_base + 0x10; portreg < maxport; portreg += 2) {
                    outw(usb_base + 0x06, 0x0); //Set the current frame number to 0
    
                    //Enable port, check to see if a device is installed
                    prints("Resetting port ");
                    printHexWord(((portreg - (0x10 + usb_base)) / 2) + 1);
                    prints("\n");
                    outw(portreg, 0x0200); //Reset port
                    prints("Waiting for port to enter reset\n");
                    while(!(inw(portreg) & 0x0200)); //Wait for reset to start
                    sleep(250); //Wait at least 64 usb times for device detection to occur (spec says at least 50, we're doing 250 to deal with flaky devices)
                    outw(portreg, 0x0000); //Clear reset
                    prints("Waiting for port to exit reset\n");
                    while(inw(portreg) & 0x0200);
                    outw(portreg, 0x0004); //Enable port
                    prints("Waiting for port to be enabled\n");
                    for(j = 0; j < 20; j++) {
                        
                        if(inw(portreg) & 0x0004)
                            break;  //Wait for enable to take hold
                    }
                    if(!(inw(portreg) & 0x0004))
                        prints("Could not enable port\n");
                    sleep(10); //Wait for the device to come online 
    
                    if(inw(portreg) & 0x0001) {
        
                        //attempt device init twice
                        for(j = 0; j < 2; j++) {
            
                            //We create a control transfer to device 0 control pipe:
                            //Create a SETUP address 0 packet TD with null next pointer, insert a reference to it into the frame list
                            prints("Setting up transfer structures\n");
                            for(j = 0; j < 0x400; j++)
                                frame_list[j] = ((unsigned int)(&usb_ram[0])) | 0x02; //All frame list pointers points to a qh at usb_ram[0] (TDs are 16-byte aligned) and has 'terminate' unmarked
                            //This is a queue head which will point to our default queue of TDs
                            usb_ram[0] = 0x01; //This is the head of our test queue, set to terminate
                            usb_ram[1] = ((unsigned int)&usb_ram[4] & 0xFFFFFFF0); // Pointer to the first TD in the list, 
                            usb_ram[4] = ((unsigned int)&usb_ram[12] & 0xFFFFFFF0) | 0x4; //This starts the TD, and points to the next TD in the list -- also marked depth-first
                            usb_ram[5] = 0x1C800000; //Transfer active, Check to see later on if 0x800000 is set. If it's not, the transaction was carried out. And if 0x18000000 = 0x18000000 it had no errors
                            usb_ram[6] = 0x00E8002D; //eight bytes data, endpoint 0, address 0, PID 0x2D (SETUP), DATA 0
                            usb_ram[7] = (unsigned int)&usb_ram[20]; //Pointer for the buffer that the controller should read the packet data from
                            usb_ram[8] = 0; usb_ram[9] = 0; usb_ram[10] = 0; usb_ram[11] = 0; //There's extra shit here, fuck if I know why
                            usb_ram[12] = 0x1; //This starts the TD and is a null terminate pointer as it will also be the last TD
                            usb_ram[13] = 0x1C800000; //Transfer active, Check to see later on if 0x800000 is set. If it's not, the transaction was carried out. And if 0x18000000 = 0x18000000 it had no errors
                            usb_ram[14] = 0x00E00069; //eight bytes data (we will be splitting the descriptor into multiple 8-byte packets), endpoint 0, address 0, PID 0x69 (IN), DATA 1
                            usb_ram[15] = (unsigned int)&usb_ram[22]; //Pointer for the buffer that the controller should read the packet data to
                            usb_ram[16] = 0; usb_ram[17] = 0; usb_ram[18] = 0; usb_ram[19] = 0; //There's extra shit here, fuck if I know why
                            usb_buf = (unsigned char*)&usb_ram[20]; //Treating the packet data buffer as a byte array to make setting it up a little easier
                            usb_buf[0] = 0x80; //(device, standard, device-to-host)
                            usb_buf[1] = 0x06; //Get descriptor
                            usb_buf[2] = 0x00; //Descriptor index 0
                            usb_buf[3] = 0x01; //Device descriptor
                            usb_buf[4] = 0x0; //wIndex = 0
                            usb_buf[5] = 0x0; //wIndex = 0
                            usb_buf[6] = 0x12; //wLength = 18 bytes //Set to 8 to match our max packet size in the setup packet
                            usb_buf[7] = 0x00; //wLength = 18 byes
                            //usb_ram[12] = 0x01000680; //bmRequestType = 0x80 (device, standard, device-to-host), bRequest = 0x06 (descriptor), wValue = 0x0100 (device descriptor/descriptor index 0)
                            //usb_ram[13] = 0x00120000; //wIndex = 0x0000 (unused in this request), wLength = 0x0012 (18 bytes, which is the length of a device descriptor)
        
        /*
        GetDvcDescrReq:
        DB SRRTIn+SRRTTypeStandard+SRRTRecipDevice = 0x80 + 0x00 + 0x00 = 0x80
        DB SRRQGetDescriptor = 0x06
        DB 0
        DB DescrTypeDevice = 0x01
        DW 0
        DW DvcDescrSize = 0x12
        */
        
                            usb_ram[22] = 0x0; //This is the buffer into which our device descriptor should be read
                            usb_ram[23] = 0x0;
                            usb_ram[24] = 0x0;
                            usb_ram[25] = 0x0;
                            usb_ram[26] = 0x0;
        
                            while(1) {
        
                                //Set frame number to 0
                                outw(usb_base + 0x06, 0x0); //Set the current frame number to 0
        
                                //Set run/stop to run
                                //Make sure that none of the interrupt flags are set, and clear them if they are
                                if(inw(usb_base + 0x02) & 0x10) {
        
                                    prints("[uhci] INTERNAL PROCESS ERROR!");
                                    while(1);
                                }
                                
                                //Set software debug on
                                outw(usb_base, inw(usb_base) | 0x20);
        
                                //We should clear HCHalted here, to make sure we're not reading our own value later on
                                outw(usb_base + 0x02, 0x20); //The usbsts register is R/WC which means that writing a one to a location resets its value to zero
                                prints("Setting host controller to run...\n");
                                outw(usb_base, inw(usb_base) | 0x0001);
                                while(inw(usb_base + 0x02) & 0x20); //Wait until the halted status clears
        
                                //Poll HCHalted until the device is halted
                                while(!(inw(usb_base + 0x02) & 0x20));
        
        
                                //Check first TD (SETUP)
                                //Display TD status
                                prints("TD 1 (SETUP)\n");
                                //Display the status of the transaction descriptor
                                prints(" (");
                                printHexDword(usb_ram[5]);
                                prints(") USBSTS: 0x");
                                printHexWord(inw(usb_base + 0x02));
                                pchar('\n');
                                if(!((usb_ram[5] & 0x800000)) && ((usb_ram[5] & 0x18000000) != 0)) {
        
                                    prints("    Setup completed successfully\n");
                                    td1 = 1;
                                }
        
                                if(!(usb_ram[5] & 0x18000000)) {
        
                                    prints("    Could not recover from transfer errors.\n");
                                    break;
                                }
        
                                //Check the second TD (IN)
                                //Display TD status
                                prints("TD 2 (IN)\n");
                                //Display the status of the transaction descriptor
                                prints(" (");
                                printHexDword(usb_ram[13]);
                                prints(") USBSTS: 0x");
                                printHexWord(inw(usb_base + 0x02));
                                pchar('\n');
                                if(!((usb_ram[13] & 0x800000)) && ((usb_ram[13] & 0x18000000) != 0)) {
        
                                    prints("    Setup completed successfully\n");
                                    printHexDword(usb_ram[22]); pchar('\n');
                                    printHexDword(usb_ram[23]); pchar('\n');
                                    printHexDword(usb_ram[24]); pchar('\n');
                                    printHexDword(usb_ram[25]); pchar('\n');
                                    printHexDword(usb_ram[26]); pchar('\n');
                                    td2 = 1;
                                    while(1); //REMOVE ME
                                }
        
                                if(!(usb_ram[13] & 0x18000000)) {
        
                                    prints("    Could not recover from transfer errors.\n");
                                    break;
                                }
        
                                if(td1 && td2)
                                    break;
        
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
                            
                            //Exit early if the first attempt worked
                            if(td1 && td2)
                                break;
                                
                            if(j)
                                prints("Second attempt failed\n");
                        }
                        
                        
                        while(1); //Temporarily hang once we're done with the first port for testing
                    } else {
    
                        outw(portreg, 0x000A); //Disable port
                        prints("No device found on port\n");
                    }
                }
            }
        }
        
        while(1); 
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
