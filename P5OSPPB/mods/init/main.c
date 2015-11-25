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
    prints("[init] Starting pci server...\n");
    startAndWait(":pci.mod");

    //Start UHCI server
    prints("[init] Starting UHCI server...\n");
    startAndWait(":uhci.mod");

    //Start WYG server
    prints("[init] Starting WYG server...\n");
    startAndWaitUsr(":wyg.mod");

    //Finally, start up the user 'login' process
    prints("[init] Servers ready. Starting user process.\n");
    startProc(":usr.mod");

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
