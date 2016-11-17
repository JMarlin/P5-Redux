#include "../include/p5.h"

void startAndWait(char* file);
void startAndWaitUsr(char* file);

//Update this in the future to read the boot module names from a text file
void main(void) {

    //Start the idle process
    prints("[init] Starting idle process...\n");
    startSuperProc(":idle.mod");

    //Start up all of the initial-boot servers
    prints("[init] Starting VESA server...\n");
    startAndWait(":vesa.mod");

    //Start PCI server
    //prints("[init] Starting pci server...\n");
    //startAndWait(":pci.mod");

    //Start UHCI server
//    prints("[init] Starting UHCI server...\n");
//    startAndWait(":uhci.mod");
    
    //Start key server
    prints("[init] Starting ps2 server...\n");
    startAndWait(":ps2.mod");

// HANGING TO TEST THE PS2 SERVICE
    //Start WYG server
    prints("[init] Starting WYG server...\n");
    startProc(":wyg.mod");
    //startAndWaitUsr(":wyg.mod");
    
    //Start UART handler
//    prints("[init] Starting UART server...\n");
//    startAndWait(":uart.mod");

    //Finally, start up the user 'login' process
    //prints("[init] Servers ready. Starting user process.\n");
    //startProc(":usr.mod");
    
    //while(1);
    terminate();
}

void startAndWaitUsr(char* file) {

    message temp_msg;
    unsigned int pid = startProc(file);

    //Send a message so the child gets the
    //parent PID, then wait for a message once
    //the child has registered itself
    postMessage(pid, 0, 0);
    getMessage(&temp_msg);
}

void startAndWait(char* file) {

    message temp_msg;
    unsigned int pid = startSuperProc(file);

    //Send a message so the child gets the
    //parent PID, then wait for a message once
    //the child has registered itself
    postMessage(pid, 0, 0);
    getMessage(&temp_msg);
}
