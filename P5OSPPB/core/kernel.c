#include "kernel.h"
#include "global.h"
#include "../memory/gdt.h"
#include "../ascii_io/ascii_i.h"
#include "../ascii_io/ascii_o.h"
#include "int.h"
#include "../timer/timer.h"


extern long _pkgoffset;
extern char _imagename;

void kernel_finish_startup(void);


int main(void) {

    __asm__ ("cli");

    initScreen();
    setColor(0x1F);
    clear();
    DEBUG("Setting up interrupt table...");
    initIDT();
    installExceptionHandlers();
    DEBUG("Done.\nSetting up the GDT...");
    initGdt();
    DEBUG("Done.\nSetting up the timer...");
    init_timer();
    timer_on();
    DEBUG("Done.\n");
    
    while(1);
}


//Start usr prompt
//NEEDS TO BE PURGED FROM THE CODEBASE
void sys_console() {

    while(1);
}
