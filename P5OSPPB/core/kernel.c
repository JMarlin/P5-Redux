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

    int key_stat;
    int tempCh = 0;

    __asm__ ("cli");

    initScreen();
    setColor(0x1F);
    clear();
    prints("Setting up interrupt table...");
    initIDT();
    installExceptionHandlers();
    prints("Done.\nSetting up the GDT...");
    initGdt();
    prints("Done.\nSetting up keyboard...");

    if((key_stat = keyboard_init()) != 1) {
        prints("Failed (");
        printHexByte((unsigned char)(key_stat & 0xFF));
        prints(")\n[P5]: No input device availible.\n");
    } else {
        prints("Done.\n");
    }

    prints("Please press enter to detect your keyboard type...");
    setupKeyTable();
    while(!(tempCh = getch()));
    if(tempCh == 'A')
        setupKeyTable_set1();

    pchar('\n');
    prints("\nInitializing process mgmt...");
    startProcessManagement();
    prints("Done.\nSetting up the timer...");
    init_timer(); //Mask all PIC channels
    //__asm__ ("sti");
    prints("Done.\nTurning on paging...");
    init_mmu();
    init_memory(&kernel_finish_startup); //We do this weirdness because init_memory
                                         //has to jump into a v86 process and back.
}

void kernel_resume_from_mips_calc(unsigned int mips);
void kernel_finish_startup(void) {

    prints("Done.\n");
    timer_on(); //Unmask timer channel
    do_mips_calc(&kernel_resume_from_mips_calc); //Will do mips calculation and return to the below function
}

void kernel_resume_from_mips_calc(unsigned int mips) {
    
    unsigned int i, doffset, *sizes;
    unsigned char *dcount;
    context* ctx;
    block_dev* ram0;

    prints("WELCOME TO P5\n");
    prints("Calculated IPS: 0x");
    printHexDword(mips);
    prints(".\n");
    throttle_timer(mips/4000); //update timer to give us 4,000 instruction time slices

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
    prints("Image: ");
    prints(&_imagename);
    prints("\nSize: ");
    printHexDword(_pkgoffset);
    prints("b\n");
    prints("Initializing filesystem\n");
    fs_init();

    //create a ramdisk device from the extents of the kernel payload
    //then install its fs driver and finally mount the ramdisk on root
    prints("Setting up ramdisk...\n");
    doffset = 0x100005 + _pkgoffset;
    ram0 = blk_ram_new(doffset, sizes[0]);
    fs_install_driver(get_ramfs_driver());
    fs_attach(FS_RAMFS, ram0, ":");
    prints("Done.\nStarting registrar.\n");

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
