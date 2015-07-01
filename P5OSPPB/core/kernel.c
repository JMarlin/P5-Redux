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
    DEBUG("Done.\nInitializing process mgmt...");
    startProcessManagement();
    init_timer();
    timer_on();
    DEBUG("Done.\nTurning on paging...");
    init_mmu();
    init_memory(&kernel_finish_startup); //We do this weirdness because init_memory
                                         //has to jump into a v86 process and back.
}

void kernel_finish_startup(void) {

    unsigned int i, doffset, *sizes;
    unsigned char *dcount;
    context* ctx;
    block_dev* ram0;
    int tempCh = 0;
    int key_stat;

    DEBUG("Done.\nSetting up keyboard...");

    if((key_stat = keyboard_init()) != 1) {
        DEBUG("Failed (");
        DEBUG_HB((unsigned char)(key_stat & 0xFF));
        DEBUG(")\n[P5]: No input device availible.\n");
    } else {
        DEBUG("Done.\n");
    }

    prints("WELCOME TO P5\n");
    prints("Please press enter to detect your keyboard type...");
    setupKeyTable();
    while(!(tempCh = getch()));
    if(tempCh == 'A')
        setupKeyTable_set1();

    pchar('\n');
    dcount = (unsigned char*)((char*)0x100000+_pkgoffset);
    sizes = (unsigned int*)((char*)0x100001+_pkgoffset);

    if(!dcount) {

        prints("No modules found.\n");
    } else {

        doffset = 5 + ((dcount[0]-1)*4);

        for(i = 0; i < dcount[0]; i++) {

            doffset += sizes[i];
        }
    }

    //Print kernel size and version
    DEBUG("Image: ");
    DEBUG(&_imagename);
    DEBUG("\nSize: ");
    DEBUG_HD(_pkgoffset);
    DEBUG("b\n");
    DEBUG("Initializing filesystem\n");
    fs_init();

    //create a ramdisk device from the extents of the kernel payload
    //then install its fs driver and finally mount the ramdisk on root
    DEBUG("Calculating offset to ramdisk\n");
    doffset = 0x100005 + _pkgoffset;
    DEBUG("Creating new ramdisk block device ram0...");
    ram0 = blk_ram_new(doffset, sizes[0]);
    DEBUG("Done\nInstalling ramfs filesystem driver...");
    fs_install_driver(get_ramfs_driver());
    DEBUG("Done\nAttaching ramfs filesystem on ram0...");
    fs_attach(FS_RAMFS, ram0, ":");

    //Start the registrar and thereby the rest of the OS
    enterProc(exec_process(":registrar.mod", 1));

    prints("PANIC: Registrar could not be started.\n");
    while(1);
}


//Start usr prompt
//NEEDS TO BE PURGED FROM THE CODEBASE
void sys_console() {

    while(1);
}
