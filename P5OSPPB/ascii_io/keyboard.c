#include "../core/global.h"
#include "../core/util.h"
#include "ascii_o.h"
#include "keyboard.h"

//NOTE: Replace all prints with debugs
void keyboard_sendCommand(unsigned char command) {

    keyboard_inputWait();
    outb(KBC_CREG, command);
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

int keyboard_init() {

    unsigned char ccbValue;
    int i;

    DEBUG("[keyboard_init()]: Beginning keyboard initialization sequence.\n");
    DEBUG("[keyboard_init()]: Disabling devices and flushing KBC buffer.\n");

    //Diable the ports
    keyboard_sendCommand(KBC_DIS_PORT1);
    keyboard_sendCommand(KBC_DIS_PORT2);

    //Flush the outbuf
    inb(KBC_DREG);

    //Set the CCB to no interrupts or translation
    DEBUG("[keyboard_init()]: Disabling device interrupts and scancode translation.\n");
    keyboard_sendCommand(KBC_READ_CCB);
    ccbValue = keyboard_getData();
    keyboard_sendCommand(KBC_WRITE_CCB);
    keyboard_sendData(ccbValue & ~(CCB_PORT1_INT | CCB_PORT2_INT | CCB_PORT1_TRANS));

    //Test the KBC
    DEBUG("[keyboard_init()]: Testing the KBC.\n");
    keyboard_sendCommand(KBC_SELFTEST);

    if(keyboard_getData() != KBC_TST_PASS) {
        DEBUG("[keyboard_init()]: KBC sleftest failed. Keyboard will be disabled.\n");
        return 2;
    }

    //Test the device
    DEBUG("[keyboard_init()]: Testing the PS/2 interface.\n");
    keyboard_sendCommand(KBC_TST_PORT1);

    if(keyboard_getData() != PORT_TST_PASS) {
        DEBUG("[keyboard_init()]: PS/2 device test failed. Keyboard will be disabled.\n");
        return 3;
    }

    //Enable and reset the device (when we start using the IRQs, we'll
    //want to enable those in the CCB here as well
    DEBUG("[keyboard_init()]: Enabling and resetting PS/2 device.\n");
    keyboard_sendCommand(KBC_READ_CCB);
    ccbValue = keyboard_getData();
    keyboard_sendCommand(KBC_WRITE_CCB);
    keyboard_sendData(ccbValue | CCB_PORT1_INT | CCB_PORT2_INT);
    keyboard_sendCommand(KBC_EN_PORT1);
    keyboard_sendData(PS2_RESET);

    if(keyboard_getData() != PS2_OK) {
        DEBUG("[keyboard_init()]: PS/2 reset failed. Keyboard will be disabled.\n");
        return 4;
    }

    //The keyboard reset sends an AA code that we need
    //to clear out of the buffer here
    keyboard_getData();

    //Should also turn scanning on and off here
    DEBUG("[keyboard_init()]: Changing keyboard scancode to set 2.\n");
    keyboard_sendData(PS2_SET_SCANCODE);
    if((ccbValue = keyboard_getData()) != PS2_OK) {

        DEBUG("[keyboard_init()]: PS/2 scancode change failed (KBC won't ACK PS2_SET_SCANCODE).\n");
        DEBUG("[keyboard_init()]: KBC Return Code: 0x");
        DEBUG_HB(ccbValue);
        DEBUG("\n");

        //halt for testing
        return 5;
    }

    keyboard_sendData(0x02); //Scancode set 2
    if((ccbValue = keyboard_getData()) != PS2_OK) {

        DEBUG("[keyboard_init()]: PS/2 scancode change failed (KBC didn't accept set number).\n");
        DEBUG("[keyboard_init()]: KBC Return Code: 0x");
        DEBUG_HB(ccbValue);
        DEBUG("\n");

        if(ccbValue != 1)
            return ccbValue;
        else
            return 6;
    }

    DEBUG("[keyboard_init()]: KBC and PS/2 device ready for use.\n");
    return 1;
}
