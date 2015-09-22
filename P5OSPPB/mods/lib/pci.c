#include "../include/pci.h"
#include "../include/p5.h"
#include "../include/registrar.h"

unsigned int pci_pid;
message temp_msg;

unsigned char pciInit() {

    //Find the PCI server
	postMessage(REGISTRAR_PID, REG_LOOKUP, SVC_PCI);
    getMessage(&temp_msg);
    pci_pid = temp_msg.payload;

    return pci_pid != 0;
}

//Simply returns the number of devices installed
unsigned int pciDeviceCount() {

    postMessage(pci_pid, PCI_DEVCOUNT, 0);
    getMessage(&temp_msg);

    return temp_msg.payload & 0xFFFF;
}

//Get the device id and vendor id
unsigned int pciGetDeviceIDs(pci_address address) {

    return pciReadField(address, 0);
}

//Get the class code, subclass, prog if and rev id
unsigned int pciGetDeviceClass(pci_address address) {

    return pciReadField(address, 2);
}

//Scrape the entirety of the device's configuration ram
pci_config pciGetDeviceConfig(pci_address address) {

    int i;
    pci_config return_config;
    unsigned int *int_array = (unsigned int*)&return_config;

    for(i = 0; i < 16; i++)
        int_array[i] = pciReadField(address, i);

    return return_config;
}

//Read a register field from the specified device
unsigned int pciReadField(pci_address address, unsigned char reg) {

    reg = reg & 0xF;
    postMessage(pci_pid, PCI_READREG + reg, address);
    getMessageFrom(&temp_msg, pci_pid, PCI_READREG + reg);

    return temp_msg.payload;
}

unsigned int pciGetBus(pci_address address) {

    postMessage(pci_pid, PCI_GETBUS, address);
    getMessageFrom(&temp_msg, pci_pid, PCI_GETBUS);

    return temp_msg.payload;
}

unsigned int pciGetDevice(pci_address address) {

    postMessage(pci_pid, PCI_GETDEV, address);
    getMessageFrom(&temp_msg, pci_pid, PCI_GETDEV);

    return temp_msg.payload;
}
unsigned int pciGetFunction(pci_address address) {

    postMessage(pci_pid, PCI_GETFUNC, address);
    getMessageFrom(&temp_msg, pci_pid, PCI_GETFUNC);

    return temp_msg.payload;
}
