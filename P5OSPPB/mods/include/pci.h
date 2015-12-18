#ifndef PCI_H
#define PCI_H

#define PCI_MSG_CLASS ((unsigned int)0x00300000)

#define PCI_DEVCOUNT (PCI_MSG_CLASS | 0x1)
#define PCI_READREG_0 (PCI_MSG_CLASS | 0x2)
#define PCI_READREG_1 (PCI_MSG_CLASS | 0x3)
#define PCI_READREG_2 (PCI_MSG_CLASS | 0x4)
#define PCI_READREG_3 (PCI_MSG_CLASS | 0x5)
#define PCI_READREG_4 (PCI_MSG_CLASS | 0x6)
#define PCI_READREG_5 (PCI_MSG_CLASS | 0x7)
#define PCI_READREG_6 (PCI_MSG_CLASS | 0x8)
#define PCI_READREG_7 (PCI_MSG_CLASS | 0x9)
#define PCI_READREG_8 (PCI_MSG_CLASS | 0xA)
#define PCI_READREG_9 (PCI_MSG_CLASS | 0xB)
#define PCI_READREG_A (PCI_MSG_CLASS | 0xC)
#define PCI_READREG_B (PCI_MSG_CLASS | 0xD)
#define PCI_READREG_C (PCI_MSG_CLASS | 0xE)
#define PCI_READREG_D (PCI_MSG_CLASS | 0xF)
#define PCI_READREG_E (PCI_MSG_CLASS | 0x10)
#define PCI_READREG_F (PCI_MSG_CLASS | 0x12)
#define PCI_READREG_10 (PCI_MSG_CLASS | 0x13)
#define PCI_READREG_11 (PCI_MSG_CLASS | 0x14)
#define PCI_READREG_12 (PCI_MSG_CLASS | 0x15)
#define PCI_READREG_13 (PCI_MSG_CLASS | 0x16)
#define PCI_READREG_14 (PCI_MSG_CLASS | 0x17)
#define PCI_READREG_15 (PCI_MSG_CLASS | 0x18)
#define PCI_READREG_16 (PCI_MSG_CLASS | 0x19)
#define PCI_READREG_17 (PCI_MSG_CLASS | 0x1A)
#define PCI_READREG_18 (PCI_MSG_CLASS | 0x1B)
#define PCI_READREG_19 (PCI_MSG_CLASS | 0x1C)
#define PCI_READREG_1A (PCI_MSG_CLASS | 0x1D)
#define PCI_READREG_1B (PCI_MSG_CLASS | 0x1E)
#define PCI_READREG_1C (PCI_MSG_CLASS | 0x1F)
#define PCI_READREG_1D (PCI_MSG_CLASS | 0x20)
#define PCI_READREG_1E (PCI_MSG_CLASS | 0x21)
#define PCI_READREG_1F (PCI_MSG_CLASS | 0x22)
#define PCI_READREG_20 (PCI_MSG_CLASS | 0x23)
#define PCI_READREG_21 (PCI_MSG_CLASS | 0x24)
#define PCI_READREG_22 (PCI_MSG_CLASS | 0x25)
#define PCI_READREG_23 (PCI_MSG_CLASS | 0x26)
#define PCI_READREG_24 (PCI_MSG_CLASS | 0x27)
#define PCI_READREG_25 (PCI_MSG_CLASS | 0x28)
#define PCI_READREG_26 (PCI_MSG_CLASS | 0x29)
#define PCI_READREG_27 (PCI_MSG_CLASS | 0x2A)
#define PCI_READREG_28 (PCI_MSG_CLASS | 0x2B)
#define PCI_READREG_29 (PCI_MSG_CLASS | 0x2C)
#define PCI_READREG_2A (PCI_MSG_CLASS | 0x2D)
#define PCI_READREG_2B (PCI_MSG_CLASS | 0x2E)
#define PCI_READREG_2C (PCI_MSG_CLASS | 0x2F)
#define PCI_READREG_2D (PCI_MSG_CLASS | 0x30)
#define PCI_READREG_2E (PCI_MSG_CLASS | 0x31)
#define PCI_READREG_2F (PCI_MSG_CLASS | 0x32)
#define PCI_READREG_30 (PCI_MSG_CLASS | 0x33)
#define PCI_GETBUS (PCI_MSG_CLASS | 0x34)
#define PCI_GETDEV (PCI_MSG_CLASS | 0x35)
#define PCI_GETFUNC (PCI_MSG_CLASS | 0x36)
#define PCI_WRITEREG_0 (PCI_MSG_CLASS | 0x37)
#define PCI_WRITEREG_1 (PCI_MSG_CLASS | 0x38)
#define PCI_WRITEREG_2 (PCI_MSG_CLASS | 0x39)
#define PCI_WRITEREG_3 (PCI_MSG_CLASS | 0x3A)
#define PCI_WRITEREG_4 (PCI_MSG_CLASS | 0x3B)
#define PCI_WRITEREG_5 (PCI_MSG_CLASS | 0x3C)
#define PCI_WRITEREG_6 (PCI_MSG_CLASS | 0x3D)
#define PCI_WRITEREG_7 (PCI_MSG_CLASS | 0x3E)
#define PCI_WRITEREG_8 (PCI_MSG_CLASS | 0x3F)
#define PCI_WRITEREG_9 (PCI_MSG_CLASS | 0x40)
#define PCI_WRITEREG_A (PCI_MSG_CLASS | 0x41)
#define PCI_WRITEREG_B (PCI_MSG_CLASS | 0x42)
#define PCI_WRITEREG_C (PCI_MSG_CLASS | 0x43)
#define PCI_WRITEREG_D (PCI_MSG_CLASS | 0x44)
#define PCI_WRITEREG_E (PCI_MSG_CLASS | 0x45)
#define PCI_WRITEREG_F (PCI_MSG_CLASS | 0x46)
#define PCI_WRITEREG_10 (PCI_MSG_CLASS | 0x47)
#define PCI_WRITEREG_11 (PCI_MSG_CLASS | 0x48)
#define PCI_WRITEREG_12 (PCI_MSG_CLASS | 0x49)
#define PCI_WRITEREG_13 (PCI_MSG_CLASS | 0x4A)
#define PCI_WRITEREG_14 (PCI_MSG_CLASS | 0x4B)
#define PCI_WRITEREG_15 (PCI_MSG_CLASS | 0x4C)
#define PCI_WRITEREG_16 (PCI_MSG_CLASS | 0x4D)
#define PCI_WRITEREG_17 (PCI_MSG_CLASS | 0x4E)
#define PCI_WRITEREG_18 (PCI_MSG_CLASS | 0x4F)
#define PCI_WRITEREG_19 (PCI_MSG_CLASS | 0x50)
#define PCI_WRITEREG_1A (PCI_MSG_CLASS | 0x51)
#define PCI_WRITEREG_1B (PCI_MSG_CLASS | 0x52)
#define PCI_WRITEREG_1C (PCI_MSG_CLASS | 0x53)
#define PCI_WRITEREG_1D (PCI_MSG_CLASS | 0x54)
#define PCI_WRITEREG_1E (PCI_MSG_CLASS | 0x55)
#define PCI_WRITEREG_1F (PCI_MSG_CLASS | 0x56)
#define PCI_WRITEREG_20 (PCI_MSG_CLASS | 0x57)
#define PCI_WRITEREG_21 (PCI_MSG_CLASS | 0x58)
#define PCI_WRITEREG_22 (PCI_MSG_CLASS | 0x59)
#define PCI_WRITEREG_23 (PCI_MSG_CLASS | 0x5A)
#define PCI_WRITEREG_24 (PCI_MSG_CLASS | 0x5B)
#define PCI_WRITEREG_25 (PCI_MSG_CLASS | 0x5C)
#define PCI_WRITEREG_26 (PCI_MSG_CLASS | 0x5D)
#define PCI_WRITEREG_27 (PCI_MSG_CLASS | 0x5E)
#define PCI_WRITEREG_28 (PCI_MSG_CLASS | 0x5F)
#define PCI_WRITEREG_29 (PCI_MSG_CLASS | 0x60)
#define PCI_WRITEREG_2A (PCI_MSG_CLASS | 0x61)
#define PCI_WRITEREG_2B (PCI_MSG_CLASS | 0x62)
#define PCI_WRITEREG_2C (PCI_MSG_CLASS | 0x63)
#define PCI_WRITEREG_2D (PCI_MSG_CLASS | 0x64)
#define PCI_WRITEREG_2E (PCI_MSG_CLASS | 0x65)
#define PCI_WRITEREG_2F (PCI_MSG_CLASS | 0x66)
#define PCI_WRITEREG_30 (PCI_MSG_CLASS | 0x67)
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
