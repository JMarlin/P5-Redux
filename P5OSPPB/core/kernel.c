#include "../process/process.h"
#include "kernel.h"
#include "global.h"
#include "../ascii_io/ascii_i.h"
#include "../ascii_io/ascii_o.h"
#include "../fat12/hiinter.h"
#include "util.h"
#include "int.h"
#include "../ascii_io/keyboard.h"
#include "../memory/memory.h"
#include "../memory/paging.h"
#include "../memory/gdt.h"
#include "../fs/fs.h"
#include "../fs/ramfs.h"
#include "../block/block.h"
#include "../block/ramdisk.h"
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
    init_timer(); //Mask all PIC channels
    __asm__ ("sti");
    timer_on(); //Unmask timer channel
    DEBUG("Done.\n");

    while(1);
}


//Start usr prompt
//NEEDS TO BE PURGED FROM THE CODEBASE
void sys_console() {

    while(1);
}
