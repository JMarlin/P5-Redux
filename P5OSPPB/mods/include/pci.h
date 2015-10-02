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
#define PCI_READREG_F 0x12
#define PCI_READREG_10 0x13
#define PCI_READREG_11 0x14
#define PCI_READREG_12 0x15
#define PCI_READREG_13 0x16
#define PCI_READREG_14 0x17
#define PCI_READREG_15 0x18
#define PCI_READREG_16 0x19
#define PCI_READREG_17 0x1A
#define PCI_READREG_18 0x1B
#define PCI_READREG_19 0x1C
#define PCI_READREG_1A 0x1D
#define PCI_READREG_1B 0x1E
#define PCI_READREG_1C 0x1F
#define PCI_READREG_1D 0x20
#define PCI_READREG_1E 0x21
#define PCI_READREG_1F 0x22
#define PCI_READREG_20 0x23
#define PCI_READREG_21 0x24
#define PCI_READREG_22 0x25
#define PCI_READREG_23 0x26
#define PCI_READREG_24 0x27
#define PCI_READREG_25 0x28
#define PCI_READREG_26 0x29
#define PCI_READREG_27 0x2A
#define PCI_READREG_28 0x2B
#define PCI_READREG_29 0x2C
#define PCI_READREG_2A 0x2D
#define PCI_READREG_2B 0x2E
#define PCI_READREG_2C 0x2F
#define PCI_READREG_2D 0x30
#define PCI_READREG_2E 0x31
#define PCI_READREG_2F 0x32
#define PCI_READREG_30 0x33
#define PCI_GETBUS 0x34
#define PCI_GETDEV 0x35
#define PCI_GETFUNC 0x36
#define PCI_WRITEREG_0 0x2
#define PCI_WRITEREG_1 0x3
#define PCI_WRITEREG_2 0x4
#define PCI_WRITEREG_3 0x5
#define PCI_WRITEREG_4 0x6
#define PCI_WRITEREG_5 0x7
#define PCI_WRITEREG_6 0x8
#define PCI_WRITEREG_7 0x9
#define PCI_WRITEREG_8 0xA
#define PCI_WRITEREG_9 0xB
#define PCI_WRITEREG_A 0xC
#define PCI_WRITEREG_B 0xD
#define PCI_WRITEREG_C 0xE
#define PCI_WRITEREG_D 0xF
#define PCI_WRITEREG_E 0x10
#define PCI_WRITEREG_F 0x12
#define PCI_WRITEREG_10 0x13
#define PCI_WRITEREG_11 0x14
#define PCI_WRITEREG_12 0x15
#define PCI_WRITEREG_13 0x16
#define PCI_WRITEREG_14 0x17
#define PCI_WRITEREG_15 0x18
#define PCI_WRITEREG_16 0x19
#define PCI_WRITEREG_17 0x1A
#define PCI_WRITEREG_18 0x1B
#define PCI_WRITEREG_19 0x1C
#define PCI_WRITEREG_1A 0x1D
#define PCI_WRITEREG_1B 0x1E
#define PCI_WRITEREG_1C 0x1F
#define PCI_WRITEREG_1D 0x20
#define PCI_WRITEREG_1E 0x21
#define PCI_WRITEREG_1F 0x22
#define PCI_WRITEREG_20 0x23
#define PCI_WRITEREG_21 0x24
#define PCI_WRITEREG_22 0x25
#define PCI_WRITEREG_23 0x26
#define PCI_WRITEREG_24 0x27
#define PCI_WRITEREG_25 0x28
#define PCI_WRITEREG_26 0x29
#define PCI_WRITEREG_27 0x2A
#define PCI_WRITEREG_28 0x2B
#define PCI_WRITEREG_29 0x2C
#define PCI_WRITEREG_2A 0x2D
#define PCI_WRITEREG_2B 0x2E
#define PCI_WRITEREG_2C 0x2F
#define PCI_WRITEREG_2D 0x30
#define PCI_WRITEREG_2E 0x31
#define PCI_WRITEREG_2F 0x32
#define PCI_WRITEREG_30 0x33
#define PCI_READREG PCI_READREG_0
#define PCI_WRITEREG PCI_WRITEREG_0

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
void pciWriteField(pci_address address, unsigned char reg, unsigned int value);

#endif //PCI_H
