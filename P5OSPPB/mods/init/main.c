#include "../include/p5.h"

void main(void) {

    message temp_msg;

    //Start up all of the initial-boot servers
    startSuperProc(":vesa.mod");

    //Finally, start up the user 'login' process
    startProc(":usr.mod");

    //And exit
    terminate();
}
