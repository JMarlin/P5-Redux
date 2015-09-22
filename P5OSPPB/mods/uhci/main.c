#include "../include/p5.h"
#include "../include/registrar.h"
#include "../include/pci.h"
#include "../include/usb.h"

void main(void) {

    unsigned int i;
	message temp_msg;
    unsigned char rev;
    unsigned int devcount;
	unsigned int parent_pid;

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
            printHexDword(pciReadField((pci_address)i, 0x8));
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
