#include "../include/p5.h"
#include "../include/registrar.h"

//KBC registers
#define KBC_DREG 0x60 //IO Port for the data register (r/w)
#define KBC_SREG 0x64 //O Port for the status register (r)
#define KBC_CREG 0x64 //I Port for the command register (w)

//KBC Status Register bit fields
#define SR_OBSTAT 0x01 //Output buffer status bit (0 empty/1 full)
#define SR_IBSTAT 0x02 //Input buffer status bit (0 empty/1 full)
#define SR_CMDDAT 0x08 //Bit selecting if dreg is for PS/2(0) or KBC(1)
#define SR_TOERR  0x40 //Bit selecting if KBC should timeout(1) or not(0)
#define SR_PARERR 0x80 //Bit selecting if KBC parity checks(1) or not(0)

//KBC Command Register command messages
#define KBC_READ_CCB    0x20 //Read the controller config byte from the KBC
#define KBC_WRITE_CCB   0x60 //Write the next byte to the CCB
#define KBC_EN_PORT2    0xA8 //Enable second PS/2 port (if supported)
#define KBC_DIS_PORT2   0xA7 //Disable second PS/2 port (if supported)
#define KBC_TST_PORT2   0xA9 //Run a self-test on second PS/2 port
#define KBC_SELFTEST    0xAA //Test the keyboard controller
#define KBC_TST_PORT1   0xAB //Run a self-test on first PS/2 port
#define KBC_DIS_PORT1   0xAD //Disable the first PS/2 port
#define KBC_EN_PORT1    0xAE //Enable the first PS/2 port
#define KBC_CPYLOW      0xC1 //Copy the low nybble of inport to SR high nybble
#define KBC_CPYHI       0xC2 //Copy the high nybble of inport to SR high nybble
#define KBC_READ_COP    0xD0 //Read the controller output port
#define KBC_WRITE_COP   0xD1 //Write next byte to controller out port
#define KBC_WRITE_PORT2 0xD4 //Write next byte to second PS/2 port
#define KBC_RESET_CPU   0xF1 //Pulses CPU reset line low for 6ms

//KBC Controller Configuration Byte bit fields
#define CCB_PORT1_INT   0x01 //Enable(1) or disable(0) PS/2 port 1 interrupt
#define CCB_PORT2_INT   0x02 //Enable(1) or disable(0) PS/2 port 2 interrupt
#define CCB_SYSFLAG     0x04 //If 1, system passed POST
#define CCB_PORT1_CLK   0x10 //Enable(1) or disable(0) PS/2 port 1 clock
#define CCB_PORT2_CLK   0x20 //Enable(1) or disable(0) PS/2 port 2 clock
#define CCB_PORT1_TRANS 0x40 //En(1) or dis(0) scancode translation

//KBC Controller Output Port bit fields
#define COP_SYSRST     0x01 //Reset the system. ALWAYS SET TO 1.
#define COP_A20        0x02 //Enable/disable the A20 gate
#define COP_PORT2_CLK  0x04 //Status of PS/2 port 2 clock line
#define COP_PORT2_DATA 0x08 //Status of PS/2 port 2 data line
#define COP_PORT1_INB  0x10 //True if there is a byte from PS/2 1 available
#define COP_PORT2_INB  0x20 //True if there is a byte from PS/2 2 available
#define COP_PORT1_CLK  0x40 //Status of PS/2 port 1 clock line
#define COP_PORT1_DATA 0x80 //Status of PS/2 port 1 data line

//KBC command return codes
#define PORT_TST_PASS   0x00 //Port 1 or 2 selftest all clear
#define PORT_TST_CLKLOW 0x01 //Clock line stuck low
#define PORT_TST_CLKHI  0x02 //Clock line stuck high
#define PORT_TST_DATLOW 0x03 //Data line stuck low
#define PORT_TST_DATHI  0x04 //Data line stuck high
#define KBC_TST_PASS    0x55 //KBC self test passed
#define KBC_TST_FAIL    0xFC //KBC self test failed

//PS/2 commands
#define PS2_RESET        0xFF //Reset the device
#define PS2_SET_SCANCODE 0xF0

//PS/2 command return codes
#define PS2_OK   0xFA //ACK or Success
#define PS2_FAIL 0xFC //Command failed

unsigned char keyTable[132] = "";

void setupKeyTable() {
    int i = 0;

    for(i=0;i<132;i++)
        keyTable[i] = 0;

    keyTable[0xE] = '`';
    keyTable[0x15] = 'Q';
    keyTable[0x16] = '1';
    keyTable[0x1A] = 'Z';
    keyTable[0x1B] = 'S';
    keyTable[0x1C] = 'A';
    keyTable[0x1D] = 'W';
    keyTable[0x1E] = '2';
    keyTable[0x21] = 'C';
    keyTable[0x22] = 'X';
    keyTable[0x23] = 'D';
    keyTable[0x24] = 'E';
    keyTable[0x25] = '4';
    keyTable[0x26] = '3';
    keyTable[0x29] = ' ';
    keyTable[0x2A] = 'V';
    keyTable[0x2B] = 'F';
    keyTable[0x2C] = 'T';
    keyTable[0x2D] = 'R';
    keyTable[0x2E] = '5';
    keyTable[0x31] = 'N';
    keyTable[0x32] = 'B';
    keyTable[0x33] = 'H';
    keyTable[0x34] = 'G';
    keyTable[0x35] = 'Y';
    keyTable[0x36] = '6';
    keyTable[0x3A] = 'M';
    keyTable[0x3B] = 'J';
    keyTable[0x3C] = 'U';
    keyTable[0x3D] = '7';
    keyTable[0x3E] = '8';
    keyTable[0x41] = ',';
    keyTable[0x42] = 'K';
    keyTable[0x43] = 'I';
    keyTable[0x44] = 'O';
    keyTable[0x45] = '0';
    keyTable[0x46] = '9';
    keyTable[0x49] = '.';
    keyTable[0x4A] = '/';
    keyTable[0x4B] = 'L';
    keyTable[0x4C] = ';';
    keyTable[0x4D] = 'P';
    keyTable[0x4E] = '-';
    keyTable[0x52] = '\'';
    keyTable[0x54] = '[';
    keyTable[0x55] = '=';
    keyTable[0x5A] = '\n';
    keyTable[0x5B] = ']';
}

void outb(unsigned short _port, unsigned char _data) {

	asm volatile ( "outb %0, %1" : : "a"(_data), "Nd"(_port) );
}


unsigned char inb(unsigned short _port) {

	unsigned char data;

	asm volatile ( "inb %1, %0" : "=a"(data) : "Nd"(_port) );
	return data;
}

unsigned char keyboard_getStatus() {

    return inb(KBC_SREG);
}

void keyboard_inputWait() {

    while(keyboard_getStatus() & SR_IBSTAT);
}

void keyboard_outputWait() {

    while(!(keyboard_getStatus() & SR_OBSTAT));
}

unsigned char keyboard_getData() {

    keyboard_outputWait();
    return inb(KBC_DREG);
}

void keyboard_sendData(unsigned char data) {

    keyboard_inputWait();
    outb(KBC_DREG, data);
}

void keyboard_sendCommand(unsigned char command) {

    keyboard_inputWait();
    outb(KBC_CREG, command);
}

void keyboard_clearBuffer() {

	while((keyboard_getStatus() & SR_OBSTAT)) {

		//Read bits into space
		inb(KBC_DREG);
	}
}

unsigned char processKey() {
	
    //Don't block if there's nothing in the buffer
    if(!(keyboard_getStatus() & SR_OBSTAT))
        return 0;

    tempData = inb(KBC_DREG);

    //Ignore 'up' keys
    if(tempData == 0xF0) {
        keyboard_getData();
        return 0;
    }

    //Convert the 'down' scancode to 
    if(tempData < 132) {
        return keyTable[tempData];
    } else {
        return 0;
    }

    //This should make realines cycle forever waiting for input
    return 0;
}

void main(void) {

	message temp_msg;
	unsigned char current_creg;
	unsigned int parent_pid;
    unsigned char tmpch;

	//Get the 'here's my pid' message from init
    getMessage(&temp_msg);
    parent_pid = temp_msg.source;
	prints("[key] Registering keyboard IRQ...");

	//Try to register the IRQ
	if(!registerIRQ(1)) {

		prints("Failed.\n");
		postMessage(parent_pid, 0, 0); //Tell the parent we're done registering
    	terminate();
	}

	//Enable interrupts on the keyboard controller
	prints("Done.\n[key] Enabling keyboard interrupts...");
	keyboard_clearBuffer();
	keyboard_sendCommand(KBC_READ_CCB);
	current_creg = keyboard_getData();
    keyboard_sendCommand(KBC_WRITE_CCB);
    keyboard_sendData(current_creg | CCB_PORT1_INT | CCB_PORT2_INT);

	prints("Done.\n");

	postMessage(parent_pid, 0, 1); //Tell the parent we're done registering

    //Init the default keymapping
    setupKeyTable();

	//Now that everything is set up, we can loop waiting for interrupts
	while(1) {

		//Clear the keyboard buffer
		keyboard_clearBuffer();
		waitForIRQ(1);
        tempch = processKey();
		if(tempch) pchar(tempch);
	}
}
