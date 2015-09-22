#include "../include/p5.h"
#include "../include/registrar.h"
#include "../include/pci.h"

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA 0xCFC

#define PCI_ADDRESS(b, d, f, r)              \
    ((((unsigned int) (b)) << 16) |          \
    (((unsigned int) ((d) & 0x1F)) << 11 ) | \
    (((unsigned int) ((f) & 0x7)) << 8 ) |   \
    ((unsigned int) (((r) << 2) & 0xFC)) |    \
    (unsigned int) 0x80000000);

#define IS_MULTIFUNCTION(x) (((x)->header_type & 0x80) != 0)
#define IS_BRIDGE(x) (((x)->header_type & 0x7F) == 0x01)
#define BRIDGE_GET_BUS_NUMBER(x) (unsigned char)(((x)->bar_2 >> 8) && 0xFF)
#define PCI_PACK_ADDR(b, d, f) ((unsigned int)((((unsigned int)(b)) << 16) | (((unsigned int)(d)) << 8) | ((unsigned int)(f))))
#define PCI_ADDR_BUS(x) ((unsigned char)(((x) >> 16) & 0xFF))
#define PCI_ADDR_DEV(x) ((unsigned char)(((x) >> 8) & 0xFF))
#define PCI_ADDR_FUNC(x) ((unsigned char)((x) & 0xFF))

unsigned char current_bus = 0;
unsigned char current_device = 0;
unsigned char current_function = 0;

pci_config current_config;
unsigned char pci_read_initialized = 0;
unsigned int device_count = 0;
unsigned int pci_device[50];

void outd(unsigned short _port, unsigned int _data) {

	__asm__ __volatile__ ("outl %%eax, %%dx" : : "d" (_port), "a" (_data));
}

unsigned int ind(unsigned short _port) {

	unsigned int data;

	__asm__ __volatile__ ("inl %%dx, %%eax" : "=a" (data) : "dN" (_port));
	return data;
}

unsigned int readPCIConfigReg(unsigned char bus, unsigned char device, unsigned char function, unsigned char reg) {

	unsigned int pci_address = PCI_ADDRESS(bus, device, function, reg);
	outd(PCI_CONFIG_ADDRESS, pci_address);
	return ind(PCI_CONFIG_DATA);
}

pci_config* readPCIConfig(unsigned char bus, unsigned char device, unsigned char function) {

	int i;
	unsigned int* int_array = (unsigned int*)&current_config;

	if(bus != current_bus || device != current_device || function != current_function || !pci_read_initialized) {

		pci_read_initialized = 1;
		current_bus = bus;
		current_device = device;
		current_function = function;

		for(i = 0; i < 16; i ++)
			int_array[i] = readPCIConfigReg(bus, device, function, i);
	}

	return &current_config;
}

void PCIClearEntries() {

	int i;

	for(i = 0; i < 50; i++)
		pci_device[i] = 0xFFFFFFFF;

	device_count = 0;
}

unsigned char address_exists(unsigned int test_addr) {

	int i;

	for(i = 0; i < device_count; i++)
		if(pci_device[i] == test_addr) return 1;

	return 0;
}

void PCIEnumerateBus(unsigned char bus) {

		pci_config* temp_config;
		unsigned char i, j;
		unsigned char multifunction = 0;
		unsigned int cur_addr;

		for(i = 0; i < 0x20; i++) {

			for(j = 0; j < 8; j++) {

				temp_config = readPCIConfig(bus, i, j);

				//Ignore blank slots
				if(temp_config->vendor_id == 0xFFFF)
					break;

				//Add the device if it hasn't already been
				cur_addr = PCI_PACK_ADDR(bus, i, j);
				if(!address_exists(cur_addr) && device_count < 50)
					pci_device[device_count++] = cur_addr;

				//Have to do this ahead of time as our config data
				//will get overwritten if we end up scanning another bus
				multifunction = IS_MULTIFUNCTION(temp_config);

				//If the device is a bridge, scan the bridged bus
				if(IS_BRIDGE(temp_config))
					PCIEnumerateBus(BRIDGE_GET_BUS_NUMBER(temp_config));

				if(!multifunction)
					break;
			}
		}
}

void printPCIConfig(unsigned char bus, unsigned char device, unsigned char function) {

	pci_config* pci_dev = readPCIConfig(bus, device, function);

	prints("[pci] Configuration block for PCI device "); printHexByte(bus);
	prints(", "); printHexByte(device); prints(", "); printHexByte(function); pchar('\n');
	prints("    VendorID: "); printHexWord(pci_dev->vendor_id);
	prints("    DeviceID: "); printHexWord(pci_dev->device_id); pchar('\n');
	prints("    Command: "); printHexWord(pci_dev->command);
	prints("    Status: "); printHexWord(pci_dev->status); pchar('\n');
	prints("    RevisionID: "); printHexByte(pci_dev->revision_id);
	prints("    ProgIF: "); printHexByte(pci_dev->prog_if);
	prints("    Subclass: "); printHexByte(pci_dev->subclass);
	prints("    Class Code: "); printHexByte(pci_dev->class_code); pchar('\n');
	prints("    Cache Line Size: "); printHexByte(pci_dev->cache_line_size);
	prints("    Latency Timer: "); printHexByte(pci_dev->latency_timer);
	prints("    Header Type: "); printHexByte(pci_dev->header_type);
	prints("    BIST: "); printHexByte(pci_dev->bist); pchar('\n');
	prints("    BAR0: "); printHexDword(pci_dev->bar_0);
	prints("    BAR1: "); printHexDword(pci_dev->bar_1); pchar('\n');
	prints("    BAR2: "); printHexDword(pci_dev->bar_2);
	prints("    BAR3: "); printHexDword(pci_dev->bar_3); pchar('\n');
	prints("    BAR4: "); printHexDword(pci_dev->bar_4);
	prints("    BAR5: "); printHexDword(pci_dev->bar_5); pchar('\n');
	prints("    Cardbus CIS Pointer: "); printHexDword(pci_dev->cardbus_cis_ptr); pchar('\n');
	prints("    Subsystem Vendor ID: "); printHexWord(pci_dev->subsys_vendor_id);
	prints("    Subsystem ID: "); printHexWord(pci_dev->subsys_id); pchar('\n');
	prints("    Expansion ROM Base Address: "); printHexDword(pci_dev->rom_base); pchar('\n');
	prints("    Capabilities Pointer: "); printHexByte(pci_dev->capabilities_ptr); pchar('\n');
	prints("    Interrupt Line: "); printHexByte(pci_dev->interrupt_line);
	prints("    Interrupt Pin: "); printHexByte(pci_dev->interrupt_pin);
	prints("    Min Grant: "); printHexByte(pci_dev->min_grant);
	prints("    Max Latency: "); printHexByte(pci_dev->max_latency); pchar('\n');
}

void main(void) {

	message temp_msg;
	unsigned int parent_pid;

	//Get the 'here's my pid' message from init
    getMessage(&temp_msg);
    parent_pid = temp_msg.source;
	prints("[pci] Starting PCI server...");

	//Register ourself with the registrar
	postMessage(REGISTRAR_PID, REG_REGISTER, SVC_PCI);
	getMessage(&temp_msg);

	if(!temp_msg.payload) {

        prints("Failed.\n");
        postMessage(REGISTRAR_PID, REG_DEREGISTER, SVC_PCI);
        postMessage(parent_pid, 0, 0); //Tell the parent we're done registering
        terminate();
    }

	prints("Done.\n");
	postMessage(parent_pid, 0, 1); //Tell the parent we're done registering

	//Scan the PCI bus and generate the device entries
	PCIClearEntries();
	PCIEnumerateBus(0);

	//Now that everything is set up, we can loop waiting for requests
	while(1) {

		getMessage(&temp_msg);

		switch(temp_msg.command) {

			case PCI_DEVCOUNT:
				postMessage(temp_msg.source, PCI_DEVCOUNT, device_count);
			break;

			case PCI_READREG_0:
			case PCI_READREG_1:
			case PCI_READREG_2:
			case PCI_READREG_3:
			case PCI_READREG_4:
			case PCI_READREG_5:
			case PCI_READREG_6:
			case PCI_READREG_7:
			case PCI_READREG_8:
			case PCI_READREG_9:
			case PCI_READREG_A:
			case PCI_READREG_B:
			case PCI_READREG_C:
			case PCI_READREG_D:
			case PCI_READREG_E:
			case PCI_READREG_F:
				if(temp_msg.payload < 50 && pci_device[temp_msg.payload] != 0xFFFFFFFF)
					postMessage(temp_msg.source, temp_msg.command,
						readPCIConfigReg(
							PCI_ADDR_BUS(pci_device[temp_msg.payload]),
							PCI_ADDR_DEV(pci_device[temp_msg.payload]),
							PCI_ADDR_FUNC(pci_device[temp_msg.payload]),
							temp_msg.command - PCI_READREG
						)
					);
				else
					postMessage(temp_msg.source, temp_msg.command, 0xFFFFFFFF);
			break;

			case PCI_GETBUS:
				if(temp_msg.payload < 50 && pci_device[temp_msg.payload] != 0xFFFFFFFF)
					postMessage(temp_msg.source, PCI_GETBUS, (unsigned int)PCI_ADDR_BUS(pci_device[temp_msg.payload]));
				else
					postMessage(temp_msg.source, PCI_GETBUS, 0xFFFFFFFF);
			break;

			case PCI_GETDEV:
				if(temp_msg.payload < 50 && pci_device[temp_msg.payload] != 0xFFFFFFFF)
					postMessage(temp_msg.source, PCI_GETDEV, (unsigned int)PCI_ADDR_DEV(pci_device[temp_msg.payload]));
				else
					postMessage(temp_msg.source, PCI_GETDEV, 0xFFFFFFFF);
			break;

			case PCI_GETFUNC:
				if(temp_msg.payload < 50 && pci_device[temp_msg.payload] != 0xFFFFFFFF)
					postMessage(temp_msg.source, PCI_GETFUNC, (unsigned int)PCI_ADDR_FUNC(pci_device[temp_msg.payload]));
				else
					postMessage(temp_msg.source, PCI_GETFUNC, 0xFFFFFFFF);
			break;

			default:
			break;
		}
	}
}
