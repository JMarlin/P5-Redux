//#include "..\..\..\SYSLIB\INCLUDE\P5SYS.H"
#include "KEYBOARD.H"


//#include "keyboard.h"

unsigned char keyTable[132];

inline void outb(unsigned short _port, unsigned char _data)
{
       asm volatile ( "outb %0, %1" : : "a"(_data), "Nd"(_port) );
}

inline unsigned char inb(unsigned short _port)
{
        unsigned char data;
        asm volatile ( "inb %1, %0" : "=a"(data) : "Nd"(_port) );
        return data;       
}

void setupKeyTable()
{
        int i;
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

void keyboard_sendCommand(unsigned char command){
        keyboard_inputWait();
        outb(KBC_CREG, command);
}

unsigned char keyboard_getStatus(){
        return inb(KBC_SREG);
}

void keyboard_inputWait(){
        while(keyboard_getStatus() & SR_IBSTAT);
}

void keyboard_outputWait(){
        while(!(keyboard_getStatus() & SR_OBSTAT));
}

unsigned char keyboard_getData(){
        keyboard_outputWait();
        return inb(KBC_DREG);        
}

void keyboard_sendData(unsigned char data){
        keyboard_inputWait();
        outb(KBC_DREG, data);
}

int keyboard_init(){

        unsigned char ccbValue;

        ////prints("[keyboard_init()]: Beginning keyboard initialization sequence.\n");
        //prints("[keyboard_init()]: Disabling devices and flushing KBC buffer.\n");        

        //Diable the ports 
        keyboard_sendCommand(KBC_DIS_PORT1);
        keyboard_sendCommand(KBC_DIS_PORT2);

        //Flush the outbuf
        inb(KBC_DREG); //Dump that shit into space

        //Set the CCB to no interrupts or translation
        //prints("[keyboard_init()]: Disabling device interrupts and scancode translation.\n");
        keyboard_sendCommand(KBC_READ_CCB);
        ccbValue = keyboard_getData();
        //prints("[keyboard_init()]: Old CCB  value was ");
        //printHexByte(ccbValue);
        //prints(".\n");
        keyboard_sendCommand(KBC_WRITE_CCB);
        keyboard_sendData(ccbValue & ~(CCB_PORT1_INT | CCB_PORT2_INT | 
                                       CCB_PORT1_TRANS));
        keyboard_sendCommand(KBC_READ_CCB);
        ccbValue = keyboard_getData();
        //prints("[keyboard_init()]: New CCB  value is ");
        //printHexByte(ccbValue);
        //prints(".\n");
        if(ccbValue & (CCB_PORT1_INT | CCB_PORT2_INT | CCB_PORT1_TRANS)){
                //prints("[keyboard_init()]: CCB could not be set.\n");
                return 0;
        }
        
        //Test the KBC 
        //prints("[keyboard_init()]: Testing the KBC.\n");
        keyboard_sendCommand(KBC_SELFTEST); 
        if(keyboard_getData() != KBC_TST_PASS) {
                //prints("[keyboard_init()]: KBC sleftest failed. Keyboard will be disabled.\n");
                return 0;
        }

        //Test the device
        //prints("[keyboard_init()]: Testing the PS/2 interface.\n");        
        keyboard_sendCommand(KBC_TST_PORT1);
        if(keyboard_getData() != PORT_TST_PASS) {
                //prints("[keyboard_init()]: PS/2 device test failed. Keyboard will be disabled.\n");
        }
        
        //Enable and reset the device (when we start using the IRQs, we'll 
        //want to enable those in the CCB here as well
        //prints("[keyboard_init()]: Enabling and resetting PS/2 device.\n");
        keyboard_sendCommand(KBC_EN_PORT1);      
        keyboard_sendData(PS2_RESET);
        if(keyboard_getData() != PS2_OK){
                //prints("[keyboard_init()]: PS/2 reset failed. Keyboard will be disabled.\n");
        }

        //prints("[keyboard_init()]: Setting the keyboard to scancode set 2.\n");
        if(!keyboard_setScancodeSet(2)){
                //prints("[keyboard_init()]: Keyboard init failed.\n");
                return 0;
        }

        keyboard_sendCommand(KBC_READ_CCB);
        ccbValue = keyboard_getData();
        //prints("[keyboard_init()]: New CCB  value is ");
        //printHexByte(ccbValue);
        //prints(".\n");

        //prints("[keyboard_init()]: KBC and PS/2 device ready for use.\n");
        return 1;
        
}

int keyboard_setScancodeSet(unsigned char setNumber){

        unsigned char mode;

        //First, see if we even need to change the code
        keyboard_sendData(PS2_SETCODESET);
        if(keyboard_getData() != 0xFA){
                //prints("[keyboard_setScancodeSet()]: Command not acknowledged.\n");
                return 0;
        }
        keyboard_sendData(0x00); //Get the current code set
        if(keyboard_getData() != 0xFA){
                //prints("[keyboard_setScancodeSet()]: Command not acknowledged.\n");
                return 0;
        }
        mode = keyboard_getData();
        //prints("[keyboard_setScancodeSet()]: Current mode is 0x");
        //printHexByte(mode);
        //prints(".\n");
        if(mode == setNumber){
                //prints("[keyboard_setScancodeSet()]: Code set already selected.\n");
                return 1;
        }
        

        //Send the change command
        keyboard_sendData(PS2_SETCODESET);
        if(keyboard_getData() != 0xFA){
                //prints("[keyboard_setScancodeSet()]: Command not acknowledged.\n");
                return 0;
        }
        keyboard_sendData(setNumber);
        if(keyboard_getData() != 0xFA){
                //prints("[keyboard_setScancodeSet()]: Command not acknowledged.\n");
                return 0;
        }

        //Now we'll test to make sure the command took
        keyboard_sendData(PS2_SETCODESET);
        if(keyboard_getData() != 0xFA){
                //prints("[keyboard_setScancodeSet()]: Command not acknowledged.\n");
                return 0;
        }
        keyboard_sendData(0x00); //Get the current code set
        if(keyboard_getData() != 0xFA){
                //prints("[keyboard_setScancodeSet()]: Command not acknowledged.\n");
                return 0;
        }
        mode = keyboard_getData();
        //prints("[keyboard_setScancodeSet()]: New mode is 0x");
        //printHexByte(mode);
        //prints(".\n");
        if(mode != setNumber){
                //prints("[keyboard_setScancodeSet()]: Code set could not be changed.\n");
                return 0;
        }

        //prints("[keyboard_setScancodeSet()]: Scancode set changed.\n");
        return 1;

}

//This file is the main entry point for the kernel in initializing and
//installing the driver module. For now, we're kind of cheating; We'll
//create a proper driver model later on, but for now there will be a hard
//hook defined in the kernel which will intercept a keypress.
//However, it's going to be pretty cool nonetheless that we could build
//multiple keyboard driver modules which can handle specific hardware
//such as dellkey versus bochskey (which is half the point of why dynamically
//loadable modules were even a priority, so that the peculiarities of the
//development machine can be dealt with without having to make integral
//changes to the kernel code which would end up being painfully tied to
//system architecture and hardly agile at all.

//One of the important symbols in a P5 module is the mname string,
//which defines to the kernel, and through that the end user, what on earth
//kind of thing we're even loading here in the first place.
unsigned char mname[] = "Dell Precision M60 keyboard driver";

//The next most critical symbol -- and the only other part of the object
//besides the actual object format itself and the mname string -- is the
//modInit function. This is the routine which is called by the kernel for
//all modules (though not executables, it should be noted) which allows them
//to do first time startup tasks such as, in the case of this driver,
//populating critical data structures and resetting the associated device.
int modInit(void) {

        //This sets up the array which maps scancodes to characters.
        //Pretty much the main change made from the original in-kernel
        //keyboard driver as the dell keyboard -- probably because it's
        //actually a USB device and is only acting as a PS/2 device via
        //USB legacy mode -- only sends scancode set 1 even when the
        //KBC is requested to, and reports that it has been successfully
        //changed to, set 2.
        setupKeyTable();

        //Init the keyboard, clearly
        //It should be noted, as this is probably the first instance of
        //prints you'll probably see in perusing this code, that, for the
        //moment, the p5sys.h defined function is just a stub. In the future,
        //functions in p5sys will refer to library functions which actually
        //marshall the provided arguments into an interrupt-driven system
        //call. Until then, modules just won't really do anything output
        //wise save for a pass/fail return value.
        if(!keyboard_init()) {
                //prints("[P5]: No input device availible.\n");
                return 0;
        }

        return 1;

}

unsigned char getch(void)
{
        unsigned char tempData;
        tempData = keyboard_getData();
        while(tempData == 0xF0){
                keyboard_getData();
                tempData = keyboard_getData();
        }
        if(tempData < 132){
                return keyTable[tempData];
        }else{
                return 0;
        }

        return 0; //This should make realines cycle forever waiting for input
}
