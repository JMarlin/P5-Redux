#include "../include/p5.h"

void startAndWait(char* file);

//Update this in the future to read the boot module names from a text file
void main(void) {

    //Start up all of the initial-boot servers
    prints("[init] Starting VESA server...\n");
    startAndWait(":vesa.mod");

    //Start PCI server
    prints("[init] Starting pci server...\n");
    startAndWait(":pci.mod");

    //Start UHCI server
    prints("[init] Starting UHCI server...\n");
    startAndWait(":uhci.mod");

    //Finally, start up the user 'login' process
    prints("[init] Servers ready. Starting user process.\n");
    startProc(":usr.mod");

    //And then spin forever and become the idle process
    //Probably a better way to do an idle process in the
    //future, but we need something that can always be
    //swapped in in case all other threads are sleeping
    //for messages, because if they're all sleeping then
    //we'll be locked into the kernel forever looping
    //through the scheduler trying to find a process to
    //swap in
    while(1);
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
