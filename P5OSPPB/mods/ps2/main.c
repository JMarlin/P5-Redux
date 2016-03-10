#include "../include/p5.h"
#include "../include/registrar.h"
#include "../include/key.h"
//#include "../include/mouse.h"

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

//Key types (keys are pretty much either control keys or character keys)
#define KEY_TYPE_CTRL 0 
#define KEY_TYPE_CHAR 1
#define KEY_TYPE_TERMINATE 2

//Control key constants
#define KCTRL_LSHIFT 0
#define KCTRL_RSHIFT 1
#define KCTRL_ESC    2
#define KCTRL_CAPS   3
#define KCTRL_LCTRL  4
#define KCTRL_LSYS   5 
#define KCTRL_LALT   6
#define KCTRL_RALT   7
#define KCTRL_RCTRL  8 
#define KCTRL_RSYS   9
#define KCTRL_RMENU  10
#define KCTRL_UP     11
#define KCTRL_DOWN   12
#define KCTRL_LEFT   13
#define KCTRL_RIGHT  14
#define KCTRL_NUMLK  15
#define KCTRL_SCRLK  16
#define KCTRL_INS    17
#define KCTRL_HOME   18
#define KCTRL_PGUP   19
#define KCTRL_PGDN   20
#define KCTRL_END    21
#define KCTRL_DEL    22
#define KCTRL_F1     23
#define KCTRL_F2     24
#define KCTRL_F3     25
#define KCTRL_F4     26
#define KCTRL_F5     27
#define KCTRL_F6     28
#define KCTRL_F7     29
#define KCTRL_F8     30
#define KCTRL_F9     31
#define KCTRL_F10    32
#define KCTRL_F11    33
#define KCTRL_F12    35

typedef struct keyInfo {
    unsigned char type; 
    unsigned char value;
    unsigned char shifted;
    unsigned char make;
} keyInfo;

unsigned char set_number = 1;
unsigned char key_buffer[256];
unsigned char read_index;
unsigned char write_index;
unsigned char buffer_full;
unsigned char buffer_empty;

keyInfo standardCodes1[] = {
    {KEY_TYPE_CHAR, 'a', 'A', 0x1E},
    {KEY_TYPE_CHAR, 'b', 'B', 0x30},
    {KEY_TYPE_CHAR, 'c', 'C', 0x2E},
    {KEY_TYPE_CHAR, 'd', 'D', 0x20},
    {KEY_TYPE_CHAR, 'e', 'E', 0x12},
    {KEY_TYPE_CHAR, 'f', 'F', 0x21},
    {KEY_TYPE_CHAR, 'g', 'G', 0x22},
    {KEY_TYPE_CHAR, 'h', 'H', 0x23},
    {KEY_TYPE_CHAR, 'i', 'I', 0x17},
    {KEY_TYPE_CHAR, 'j', 'J', 0x24},
    {KEY_TYPE_CHAR, 'k', 'K', 0x25},
    {KEY_TYPE_CHAR, 'l', 'L', 0x26},
    {KEY_TYPE_CHAR, 'm', 'M', 0x32},
    {KEY_TYPE_CHAR, 'n', 'N', 0x31},
    {KEY_TYPE_CHAR, 'o', 'O', 0x18},
    {KEY_TYPE_CHAR, 'p', 'P', 0x19},
    {KEY_TYPE_CHAR, 'q', 'Q', 0x10},
    {KEY_TYPE_CHAR, 'r', 'R', 0x13},
    {KEY_TYPE_CHAR, 's', 'S', 0x1F},
    {KEY_TYPE_CHAR, 't', 'T', 0x14},
    {KEY_TYPE_CHAR, 'u', 'U', 0x16},
    {KEY_TYPE_CHAR, 'v', 'V', 0x2F},
    {KEY_TYPE_CHAR, 'w', 'W', 0x11},
    {KEY_TYPE_CHAR, 'x', 'X', 0x2D},
    {KEY_TYPE_CHAR, 'y', 'Y', 0x15},
    {KEY_TYPE_CHAR, 'z', 'Z', 0x2C},
    {KEY_TYPE_CHAR, '0', ')', 0x0B},
    {KEY_TYPE_CHAR, '1', '!', 0x02},
    {KEY_TYPE_CHAR, '2', '@', 0x03},
    {KEY_TYPE_CHAR, '3', '#', 0x04},
    {KEY_TYPE_CHAR, '4', '$', 0x05},
    {KEY_TYPE_CHAR, '5', '%', 0x06},
    {KEY_TYPE_CHAR, '6', '^', 0x07},
    {KEY_TYPE_CHAR, '7', '&', 0x08},
    {KEY_TYPE_CHAR, '8', '*', 0x09},
    {KEY_TYPE_CHAR, '9', '(', 0x0A},
    {KEY_TYPE_CHAR, '`', '~', 0x29},
    {KEY_TYPE_CHAR, '-', '_', 0x0C},
    {KEY_TYPE_CHAR, '=', '+', 0x0D},
    {KEY_TYPE_CHAR, '\\', '|', 0x2B},
    {KEY_TYPE_CHAR, '\b', '\b', 0x0E},
    {KEY_TYPE_CHAR, ' ', ' ', 0x39},
    {KEY_TYPE_CHAR, '\t', '\t', 0x0F},
    {KEY_TYPE_CTRL, KCTRL_CAPS, 0, 0x3A},
    {KEY_TYPE_CTRL, KCTRL_LSHIFT, 0, 0x2A},
    {KEY_TYPE_CTRL, KCTRL_LCTRL, 0, 0x1D},
    {KEY_TYPE_CTRL, KCTRL_LALT, 0, 0x38},
    {KEY_TYPE_CTRL, KCTRL_RSHIFT, 0, 0x36},
    {KEY_TYPE_CHAR, '\n', '\n', 0x1C},
    {KEY_TYPE_CTRL, KCTRL_ESC, 0, 0x01},
    {KEY_TYPE_CTRL, KCTRL_F1, 0, 0x3B},
    {KEY_TYPE_CTRL, KCTRL_F2, 0, 0x3C},
    {KEY_TYPE_CTRL, KCTRL_F3, 0, 0x3D},
    {KEY_TYPE_CTRL, KCTRL_F4, 0, 0x3E},
    {KEY_TYPE_CTRL, KCTRL_F5, 0, 0x3F},
    {KEY_TYPE_CTRL, KCTRL_F6, 0, 0x40},
    {KEY_TYPE_CTRL, KCTRL_F7, 0, 0x41},
    {KEY_TYPE_CTRL, KCTRL_F8, 0, 0x42},
    {KEY_TYPE_CTRL, KCTRL_F9, 0, 0x43},
    {KEY_TYPE_CTRL, KCTRL_F10, 0, 0x44},
    {KEY_TYPE_CTRL, KCTRL_F11, 0, 0x57},
    {KEY_TYPE_CTRL, KCTRL_F12, 0, 0x58},
    {KEY_TYPE_CTRL, KCTRL_SCRLK, 0, 0x46},
    {KEY_TYPE_CHAR, '[', '{', 0x1A},
    {KEY_TYPE_CTRL, KCTRL_NUMLK, 0, 0x45},
    {KEY_TYPE_CHAR, ']', '}', 0x1B},
    {KEY_TYPE_CHAR, ';', ':', 0x27},
    {KEY_TYPE_CHAR, '\'', '\"', 0x28},
    {KEY_TYPE_CHAR, ',', '<', 0x33},
    {KEY_TYPE_CHAR, '.', '>', 0x34},
    {KEY_TYPE_CHAR, '/', '?', 0x35},
    {KEY_TYPE_TERMINATE, 0, 0, 0}
};

keyInfo levelTwoCodes1[] = {
    {KEY_TYPE_CTRL, KCTRL_LSYS, 0, 0x5B},
    {KEY_TYPE_CTRL, KCTRL_RCTRL, 0, 0x1D},
    {KEY_TYPE_CTRL, KCTRL_RSYS, 0, 0x5C},
    {KEY_TYPE_CTRL, KCTRL_RALT, 0, 0x38},
    {KEY_TYPE_CTRL, KCTRL_RMENU, 0, 0x5D},
    {KEY_TYPE_CTRL, KCTRL_INS, 0, 0x52},
    {KEY_TYPE_CTRL, KCTRL_HOME, 0, 0x47},
    {KEY_TYPE_CTRL, KCTRL_PGUP, 0, 0x49},
    {KEY_TYPE_CTRL, KCTRL_DEL, 0, 0x53},
    {KEY_TYPE_CTRL, KCTRL_END, 0, 0x4F},
    {KEY_TYPE_CTRL, KCTRL_PGDN, 0, 0x51},
    {KEY_TYPE_CTRL, KCTRL_UP, 0, 0x48},
    {KEY_TYPE_CTRL, KCTRL_LEFT, 0, 0x4B},
    {KEY_TYPE_CTRL, KCTRL_DOWN, 0, 0x50},
    {KEY_TYPE_CTRL, KCTRL_RIGHT, 0, 0x4D},
    {KEY_TYPE_TERMINATE, 0, 0, 0}
};

keyInfo standardCodes2[] = {
    {KEY_TYPE_CHAR, 'a', 'A', 0x1C},
    {KEY_TYPE_CHAR, 'b', 'B', 0x32},
    {KEY_TYPE_CHAR, 'c', 'C', 0x21},
    {KEY_TYPE_CHAR, 'd', 'D', 0x23},
    {KEY_TYPE_CHAR, 'e', 'E', 0x24},
    {KEY_TYPE_CHAR, 'f', 'F', 0x2B},
    {KEY_TYPE_CHAR, 'g', 'G', 0x34},
    {KEY_TYPE_CHAR, 'h', 'H', 0x33},
    {KEY_TYPE_CHAR, 'i', 'I', 0x43},
    {KEY_TYPE_CHAR, 'j', 'J', 0x3B},
    {KEY_TYPE_CHAR, 'k', 'K', 0x42},
    {KEY_TYPE_CHAR, 'l', 'L', 0x4B},
    {KEY_TYPE_CHAR, 'm', 'M', 0x3A},
    {KEY_TYPE_CHAR, 'n', 'N', 0x31},
    {KEY_TYPE_CHAR, 'o', 'O', 0x44},
    {KEY_TYPE_CHAR, 'p', 'P', 0x4D},
    {KEY_TYPE_CHAR, 'q', 'Q', 0x15},
    {KEY_TYPE_CHAR, 'r', 'R', 0x2D},
    {KEY_TYPE_CHAR, 's', 'S', 0x1B},
    {KEY_TYPE_CHAR, 't', 'T', 0x2C},
    {KEY_TYPE_CHAR, 'u', 'U', 0x3C},
    {KEY_TYPE_CHAR, 'v', 'V', 0x2A},
    {KEY_TYPE_CHAR, 'w', 'W', 0x1D},
    {KEY_TYPE_CHAR, 'x', 'X', 0x22},
    {KEY_TYPE_CHAR, 'y', 'Y', 0x35},
    {KEY_TYPE_CHAR, 'z', 'Z', 0x1A},
    {KEY_TYPE_CHAR, '0', ')', 0x45},
    {KEY_TYPE_CHAR, '1', '!', 0x16},
    {KEY_TYPE_CHAR, '2', '@', 0x1E},
    {KEY_TYPE_CHAR, '3', '#', 0x26},
    {KEY_TYPE_CHAR, '4', '$', 0x25},
    {KEY_TYPE_CHAR, '5', '%', 0x2E},
    {KEY_TYPE_CHAR, '6', '^', 0x36},
    {KEY_TYPE_CHAR, '7', '&', 0x3D},
    {KEY_TYPE_CHAR, '8', '*', 0x3E},
    {KEY_TYPE_CHAR, '9', '(', 0x46},
    {KEY_TYPE_CHAR, '`', '~', 0x0E},
    {KEY_TYPE_CHAR, '-', '_', 0x4E},
    {KEY_TYPE_CHAR, '=', '+', 0x55},
    {KEY_TYPE_CHAR, '\\', '|', 0x5D},
    {KEY_TYPE_CHAR, '\b', '\b', 0x66},
    {KEY_TYPE_CHAR, ' ', ' ', 0x29},
    {KEY_TYPE_CHAR, '\t', '\t', 0x0D},
    {KEY_TYPE_CTRL, KCTRL_CAPS, 0, 0x58},
    {KEY_TYPE_CTRL, KCTRL_LSHIFT, 0, 0x12},
    {KEY_TYPE_CTRL, KCTRL_LCTRL, 0, 0x14},
    {KEY_TYPE_CTRL, KCTRL_LALT, 0, 0x11},
    {KEY_TYPE_CTRL, KCTRL_RSHIFT, 0, 0x59},
    {KEY_TYPE_CHAR, '\n', '\n', 0x5A},
    {KEY_TYPE_CTRL, KCTRL_ESC, 0, 0x76},
    {KEY_TYPE_CTRL, KCTRL_F1, 0, 0x05},
    {KEY_TYPE_CTRL, KCTRL_F2, 0, 0x06},
    {KEY_TYPE_CTRL, KCTRL_F3, 0, 0x04},
    {KEY_TYPE_CTRL, KCTRL_F4, 0, 0x0C},
    {KEY_TYPE_CTRL, KCTRL_F5, 0, 0x03},
    {KEY_TYPE_CTRL, KCTRL_F6, 0, 0x0B},
    {KEY_TYPE_CTRL, KCTRL_F7, 0, 0x83},
    {KEY_TYPE_CTRL, KCTRL_F8, 0, 0x0A},
    {KEY_TYPE_CTRL, KCTRL_F9, 0, 0x01},
    {KEY_TYPE_CTRL, KCTRL_F10, 0, 0x09},
    {KEY_TYPE_CTRL, KCTRL_F11, 0, 0x78},
    {KEY_TYPE_CTRL, KCTRL_F12, 0, 0x07},
    {KEY_TYPE_CTRL, KCTRL_SCRLK, 0, 0x7E},
    {KEY_TYPE_CHAR, '[', '{', 0x54},
    {KEY_TYPE_CTRL, KCTRL_NUMLK, 0, 0x77},
    {KEY_TYPE_CHAR, ']', '}', 0x5B},
    {KEY_TYPE_CHAR, ';', ':', 0x4C},
    {KEY_TYPE_CHAR, '\'', '\"', 0x52},
    {KEY_TYPE_CHAR, ',', '<', 0x41},
    {KEY_TYPE_CHAR, '.', '>', 0x49},
    {KEY_TYPE_CHAR, '/', '?', 0x4A},
    {KEY_TYPE_TERMINATE, 0, 0, 0}
};

keyInfo levelTwoCodes2[] = {
    {KEY_TYPE_CTRL, KCTRL_LSYS, 0, 0x1F},
    {KEY_TYPE_CTRL, KCTRL_RCTRL, 0, 0x14},
    {KEY_TYPE_CTRL, KCTRL_RSYS, 0, 0x27},
    {KEY_TYPE_CTRL, KCTRL_RALT, 0, 0x11},
    {KEY_TYPE_CTRL, KCTRL_RMENU, 0, 0x2F},
    {KEY_TYPE_CTRL, KCTRL_INS, 0, 0x70},
    {KEY_TYPE_CTRL, KCTRL_HOME, 0, 0x6C},
    {KEY_TYPE_CTRL, KCTRL_PGUP, 0, 0x7D},
    {KEY_TYPE_CTRL, KCTRL_DEL, 0, 0x71},
    {KEY_TYPE_CTRL, KCTRL_END, 0, 0x69},
    {KEY_TYPE_CTRL, KCTRL_PGDN, 0, 0x7A},
    {KEY_TYPE_CTRL, KCTRL_UP, 0, 0x75},
    {KEY_TYPE_CTRL, KCTRL_LEFT, 0, 0x6B},
    {KEY_TYPE_CTRL, KCTRL_DOWN, 0, 0x72},
    {KEY_TYPE_CTRL, KCTRL_RIGHT, 0, 0x74},
    {KEY_TYPE_TERMINATE, 0, 0, 0}
};

#define SCANSTATE_DEFAULT 0
#define SCANSTATE_BREAK 1

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

unsigned char keyboard_hasData() {
    
    return !!(keyboard_getStatus() & SR_OBSTAT);
}

void keyboard_outputWait() {

    while(!keyboard_hasData());
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

keyInfo* findCode(keyInfo* key_collection, unsigned char code) {
    
    int i;
    
    for(i = 0; key_collection[i].type != KEY_TYPE_TERMINATE; i++) {
            
        if(key_collection[i].make == code)
            return &key_collection[i];
    }
    
    return (keyInfo*)0;
}

keyInfo* processSet1Code(unsigned char code, unsigned char* was_break) {
    
    static keyInfo* currentSet = standardCodes1; 
    keyInfo* returnCode = (keyInfo*)0;
    
    was_break[0] = 0;
    

    if(code == 0xE0) {
        
        currentSet = levelTwoCodes1;
    } else {
        
        if(code & 0x80) {
        
            was_break[0] = 1;
            code &= 0x7F;        
        }
        
        returnCode = findCode(currentSet, code);
        currentSet = standardCodes1;
    } 
    
    return returnCode;
}

keyInfo* processSet2Code(unsigned char code, unsigned char* was_break) {
    
    static unsigned char state = SCANSTATE_DEFAULT;
    static keyInfo* currentSet = standardCodes2; 
    keyInfo* returnCode = (keyInfo*)0;
    
    was_break[0] = 0;
        
    switch(state) {
          
        case SCANSTATE_DEFAULT:
            if(code == 0xF0) {
                
                state = SCANSTATE_BREAK;
            } else if(code == 0xE0) {
                
                currentSet = levelTwoCodes2;
            } else {
                
                returnCode = findCode(currentSet, code);
                currentSet = standardCodes2;
            } 
        break;
         
        case SCANSTATE_BREAK:
        
            returnCode = findCode(currentSet, code);
            currentSet = standardCodes2;
            was_break[0] = 1;
            state = SCANSTATE_DEFAULT;
        break;
    }
    
    return returnCode;
}

keyInfo* processNextCode(unsigned char* was_break) {
    
    unsigned char tempData;
        
    //Don't block if there's nothing in the buffer
    if(!(keyboard_getStatus() & SR_OBSTAT))
        return (keyInfo*)0;

    tempData = inb(KBC_DREG);
    
    if(!tempData)
        return (keyInfo*)0;
    
    if(set_number == 2) 
        return processSet2Code(tempData, was_break);
    else
        return processSet1Code(tempData, was_break);
}

void initKeyBuffer() {
    
    int i;
    
    for(i = 0; i < 256; i++)
        key_buffer[i] = 0;
        
    read_index = 0;
    write_index = 0;
    buffer_full = 0;
    buffer_empty = 1;
}

unsigned char buffer_retrieve() {
    
    unsigned char c;
    
    if(read_index == write_index)
        return 0;
            
    c = key_buffer[read_index++];
}

void buffer_insert(unsigned char c) {
    
    if((write_index == (read_index - 1)) || c == 0)
        return;
        
    key_buffer[write_index++] = c;
}

unsigned char capitalize(unsigned char c) {
    
    if(c >= 'a' && c <= 'z')
        return c - 0x20;
        
    return c;
}

void detectScancodeSet() {
    
    keyboard_clearBuffer();
    keyboard_sendData(PS2_SET_SCANCODE);
    keyboard_sendData(0);
    
    //Wait for an ACK
    while(keyboard_getData() != PS2_OK);
    
    //Get the current scancode
    if(keyboard_getData() == 2) {
        
        prints("Using scancode set 2\n");
        set_number = 2;
    } else {
        
        prints("Using scancode set 1\n");
    }
}

//This thread will go to sleep waiting for the OS to wake it on a triggered IRQ 
//and then will interpret the codes found in the PS2 keybuffer into ascii data 
//and put it into the keyboard ring buffer
void keyIRQThread() {
	
	unsigned char shift_count = 0;
    unsigned char caps = 0;
    unsigned char was_break = 0;
    keyInfo* temp_key;
	
	prints("[PS2] Started interrupt thread\n");
	prints("[PS2] Registering keyboard IRQ...");

	//Try to register the IRQ
	if(!registerIRQ(1)) {

		prints("Failed.\n");
    	terminate();
	}
	
	prints("Done.\n");
	
	while(1) {

		waitForIRQ(1);    
        
        while(keyboard_hasData()) {
            
            if(temp_key = processNextCode(&was_break)) {
                
                if(was_break) {
                    
                    if(temp_key->type == KEY_TYPE_CTRL) {
                        
                        switch(temp_key->value) {
                            
                            case KCTRL_LSHIFT:
                            case KCTRL_RSHIFT:
                                shift_count--;
                            break;
                            
                            default: 
                            break;
                        }    
                    }
                } else {
                    
                    if(temp_key->type == KEY_TYPE_CHAR) {
                    
                        if(shift_count)
                            buffer_insert(temp_key->shifted);
                        else if(caps)
                            buffer_insert(capitalize(temp_key->value));
                        else
                            buffer_insert(temp_key->value);
                    } else {
                        
                        switch(temp_key->value) {
                            
                            case KCTRL_LSHIFT:
                            case KCTRL_RSHIFT:
                                shift_count++;
                            break;
                            
                            case KCTRL_CAPS:
                                caps = !caps;
                            break;
                            
                            default: 
                            break;
                        }
                    }
                }
            }
        }
	}
}

void keyMessageThread() {
    
    message temp_msg;
    unsigned char c;
    
    prints("[PS2] Started key message thread\n");
    
    //First thing, register as a KEY service with the registrar
    //We do this here so that we have this thread's PID instead of the parent's 
    postMessage(REGISTRAR_PID, REG_REGISTER, SVC_KEY);
    getMessage(&temp_msg);
    prints("[PS2] Key service registered.\n");
    
    //We should make sure the registration works, but for now we're just assuming it did
    
    while(1) {
        
        getMessage(&temp_msg);
        
        if(temp_msg.command == KEY_GETCH) {
            
            //Wait until there's a character in the buffer 
            while(!(c = buffer_retrieve()));
            
            //Once we have one, post it back
            postMessage(temp_msg.source, KEY_GETCH, (unsigned int)c);
        }
    }
}


void mouseMessageThread() {
    /* For now, this thread does nothing
    message temp_msg;
    unsigned char c;
    
    prints("[PS2] Started mouse message thread\n");
    
    //First thing, register as a KEY service with the registrar
    //We do this here so that we have this thread's PID instead of the parent's 
    postMessage(REGISTRAR_PID, REG_REGISTER, SVC_MOUSE);
    getMessage(&temp_msg);
    prints("[PS2] Mouse service registered.");
    
    //We should make sure the registration works, but for now we're just assuming it did
    
    while(1) {
        
        getMessage(&temp_msg);
        
        if(temp_msg.command == MOUSE_GETEVENT) {
            
            //Wait until there's a character in the buffer 
            while(!(c = buffer_retrieve()));
            
            //Once we have one, post it back
            postMessage(temp_msg.source, KEY_GETCH, (unsigned int)c);
        }
    }
	*/
	
	terminate();
}


void mouseIRQThread() {
	
	unsigned char shift_count = 0;
    unsigned char caps = 0;
    unsigned char was_break = 0;
    keyInfo* temp_key;
	
	prints("[PS2] Started mouse interrupt thread\n");
	prints("[PS2] Registering mouse IRQ...");

	//Try to register the IRQ
	if(!registerIRQ(12)) {

		prints("Failed.\n");
    	terminate();
	}
	
	prints("Done.\n");
	
	while(1) {

		waitForIRQ(12);    
		prints("M");
	}
}


void main(void) {
	
	message temp_msg;
	unsigned char current_creg;
	unsigned int parent_pid;

	//Get the 'here's my pid' message from init
    getMessage(&temp_msg);
    parent_pid = temp_msg.source;

	//Enable interrupts on the keyboard controller
	prints("[PS2] Enabling keyboard interrupts...");
	keyboard_clearBuffer();
	keyboard_sendCommand(KBC_READ_CCB);
	current_creg = keyboard_getData();
    keyboard_sendCommand(KBC_WRITE_CCB);
    keyboard_sendData(current_creg | CCB_PORT1_INT | CCB_PORT2_INT);
    
    //Detect the scancode set the keyboard is using
    detectScancodeSet();
    
    //Clear the keyboard buffer
    keyboard_clearBuffer();
    
    //Initialize the key buffer which will store the received key data
    initKeyBuffer();

	prints("Done.\n");

	//Start the thread that will listen for keyboard client requests
    if(!startThread())
        keyMessageThread();

    //Start the thread that will listen for keyboard interrupts 
    if(!startThread())
        keyIRQThread();
		
	//Start the thread that will listen for mouse interrupts 
    if(!startThread())
        mouseIRQThread();
		
	//Start the thread that will listen for mouse client requests
    if(!startThread())
        mouseMessageThread();

    postMessage(parent_pid, 0, 1); //Tell the parent we're done registering

    //With all of the threads started, the original core thread can exit
	terminate();
}
