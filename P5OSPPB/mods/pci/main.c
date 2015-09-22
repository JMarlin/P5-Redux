#include "../include/p5.h"
#include "../include/registrar.h"

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA 0xCFC

#define IS_MULTIFUNCTION(x) (((x)->header_type & 0x80) != 0)
#define IS_BRIDGE(x) (((x)->header_type & 0x7F) == 0x01)
#define BRIDGE_GET_BUS_NUMBER(x) (unsigned char)(((x)->bar_2 >> 8) && 0xFF)

unsigned char current_bus = 0;
unsigned char current_device = 0;
unsigned char current_function = 0;

typedef struct  __attribute__((__packed__)) pci_config {
	unsigned short vendor_id;
	unsigned short device_id;
	unsigned short command;
	unsigned short status;
	unsigned char revision_id;
	unsigned char prog_if;
	unsigned char subclass;
	unsigned char class_code;
	unsigned char cache_line_size;
	unsigned char latency_timer;
	unsigned char header_type;
	unsigned char bist;
	unsigned int bar_0;
	unsigned int bar_1;
	unsigned int bar_2;
	unsigned int bar_3;
	unsigned int bar_4;
	unsigned int bar_5;
	unsigned int cardbus_cis_ptr;
	unsigned short subsys_vendor_id;
	unsigned short subsys_id;
	unsigned int rom_base;
	unsigned char capabilities_ptr;
	unsigned char reserved[7];
	unsigned char interrupt_line;
	unsigned char interrupt_pin;
	unsigned char min_grant;
	unsigned char max_latency;
} pci_config;

pci_config current_config;
unsigned char pci_read_initialized = 0;

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

unsigned int readPCIConfigReg(unsigned char bus, unsigned char device, unsigned char function, unsigned char reg) {

	unsigned int pci_address = (((unsigned int) bus) << 16) |
							   (((unsigned int) (device & 0x1F)) << 11 ) |
							   (((unsigned int) (function & 0x7)) << 8 ) |
							   ((unsigned int) (reg & 0xFC)) |
							   (unsigned int) 0x80000000;

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
			int_array[i] = readPCIConfigReg(bus, device, function, i << 2);
	}

	return &current_config;
}

void PCIPrintConfig(unsigned char bus, unsigned char dev, unsigned char func, pci_config* config, unsigned char indent) {

	int i;

	//Indent the line
	for(i = 0; i < indent; i++)
		pchar(' ');

	//Print the PCI address
	pchar('(');
	printHexByte(bus);
	pchar(',');
	printHexByte(dev);
	pchar(',');
	printHexByte(func);
	pchar(')');

	//Print the device summary
	prints(" VID: ");
	printHexWord(config->vendor_id);
	prints(", DID: ");
	printHexWord(config->device_id);
	prints(", CC: ");
	printHexByte(config->class_code);
	prints(", SC: ");
	printHexByte(config->subclass);
	prints(", PIF: ");
	printHexByte(config->prog_if);
	prints(", REV: ");
	printHexByte(config->revision_id);
	pchar('\n');
}

void PCIScanBus(unsigned char bus, unsigned char indent) {

		pci_config* temp_config;
		unsigned char i, j;
		unsigned char multifunction = 0;

		for(i = 0; i < 0x20; i++) {

			for(j = 0; j < 8; j++) {

				temp_config = readPCIConfig(bus, i, j);

				//Ignore blank slots
				if(temp_config->vendor_id == 0xFFFF)
					break;

				//Have to do this ahead of time as our config data
				//will get overwritten if we end up scanning another bus
				multifunction = IS_MULTIFUNCTION(temp_config);

				//Print this config of the current device
				PCIPrintConfig(bus, i, j, temp_config, indent);

				//If the device is a bridge, scan the bridged bus
				if(IS_BRIDGE(temp_config))
					PCIScanBus(BRIDGE_GET_BUS_NUMBER(temp_config), indent + 1);

				if(!multifunction)
					break;
			}
		}
}

void PCIEnumerateBus(unsigned char bus) {

	PCIScanBus(bus, 0);
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
	unsigned char current_creg;
	unsigned int parent_pid;

	//Get the 'here's my pid' message from init
    getMessage(&temp_msg);
    parent_pid = temp_msg.source;
	prints("[pci] Starting PCI server...");

	//For debug purposes, just dump the first PCI entry and hang the system
	pchar('\n');
	PCIEnumerateBus(0);
	while(1);

	postMessage(parent_pid, 0, 1); //Tell the parent we're done registering
	prints("Done.\n");

	//Now that everything is set up, we can loop waiting for requests
	while(1) {

		getMessage(&temp_msg);

		switch(temp_msg.command) {


		}
	}
}
