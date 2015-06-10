#include "../include/p5.h"

void startAndWait(char* file);

void main(void) {

    //Start up all of the initial-boot servers
    startAndWait(":vesa.mod");

    //Finally, start up the user 'login' process
    startProc(":usr.mod");

    //And exit
    terminate();
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
