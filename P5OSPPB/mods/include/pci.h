#ifndef PCI_H
#define PCI_H

#define SVC_PCI 2

#define PCI_DEVCOUNT 0x1
#define PCI_READREG_0 0x2
#define PCI_READREG_1 0x3
#define PCI_READREG_2 0x4
#define PCI_READREG_3 0x5
#define PCI_READREG_4 0x6
#define PCI_READREG_5 0x7
#define PCI_READREG_6 0x8
#define PCI_READREG_7 0x9
#define PCI_READREG_8 0xA
#define PCI_READREG_9 0xB
#define PCI_READREG_A 0xC
#define PCI_READREG_B 0xD
#define PCI_READREG_C 0xE
#define PCI_READREG_D 0xF
#define PCI_READREG_E 0x10
#define PCI_READREG_F 0x11
#define PCI_GETBUS 0x12
#define PCI_GETDEV 0x13
#define PCI_GETFUNC 0x14
#define PCI_READREG PCI_READREG_0

typedef unsigned int pci_address;

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

unsigned char pciInit();
unsigned int pciGetBus(pci_address address);
unsigned int pciGetDevice(pci_address address);
unsigned int pciGetFunction(pci_address address);
unsigned int pciDeviceCount();
unsigned int pciGetDeviceIDs(pci_address address);
unsigned int pciGetDeviceClass(pci_address address);
pci_config pciGetDeviceConfig(pci_address address);
unsigned int pciReadField(pci_address address, unsigned char reg);

#endif //PCI_H
