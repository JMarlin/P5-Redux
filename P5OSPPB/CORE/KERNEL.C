#include "../process/process.h"
#include "kernel.h"
#include "../ascii_io/ascii_i.h"
#include "../ascii_io/ascii_o.h"
#include "cicomp.h"
#include "../fat12/hiinter.h"
#include "commands.h"
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


extern long pkgoffset;
extern char imagename;
char prompt[] = "P5-> ";
char inbuf[50];
char* usrBase = (char*)0x801000;
char* usrCode = (char*)0x80000;

int main(void) {

    unsigned int i, doffset, *sizes;
    unsigned char *dcount;
    context* ctx;
    block_dev* ram0;
    unsigned char listBuf[256];
    

    initScreen();
    setColor(0x1F);
    clear();

/* fix this
  if(!enableA20())
  {
        prints("FATAL ERROR: Could not enable the A20 line.\nP5 will now halt.");
        while(1);
  }
*/
    prints("Setting up interrupt table...");
    initIDT();
    installExceptionHandlers();
    prints("Done.\nSetting up the GDT...");
    initGdt();
    prints("Done.\nTurning on paging...");
    initMMU();
    prints("Done.\nSetting up keyboard...");

    if(!keyboard_init())
        prints("Failed\n[P5]: No input device availible.\n");
    else
        prints("Done.\n");

    init_memory();        
    setupKeyTable();
    prints("WELCOME TO P5\n");

    //ksize = (unsigned int*)0x100005;

    dcount = (unsigned char*)((char*)0x100000+pkgoffset);
    sizes = (unsigned int*)((char*)0x100001+pkgoffset);

    if(!dcount) {

        prints("No modules found.\n");
    } else {

        doffset = 5 + ((dcount[0]-1)*4);
        for(i = 0; i < dcount[0]; i++) {
            prints("0x");
            printHexDword(sizes[i]);
            prints(" byte module found at image+0x");
            printHexDword(doffset);
            pchar('\n');
            doffset += sizes[i];
        }
    }

    //Print kernel size and version
    prints("Image: ");
    prints(&imagename);
    prints("\nSize: ");
    printHexDword(pkgoffset);
    prints("b\n");

    /*
    //Test V86 mode
    prints("Installing V86 code...");
    usrCode[0] = 0xB8;
    usrCode[1] = 0x34;
    usrCode[2] = 0x12;
    usrCode[3] = 0xCC;
    usrCode[4] = 0xB8;
    usrCode[5] = 0x13;
    usrCode[6] = 0x00;
    usrCode[7] = 0xCD;
    usrCode[8] = 0x10;
    usrCode[9] = 0xB8;
    usrCode[10] = 0x00;
    usrCode[11] = 0x00;
    usrCode[12] = 0xCD;
    usrCode[13] = 0xFF;
    prints("Done.\n");
    prints("Entering V86 mode\n");
    ctx = newV86Proc();
    setProcEntry(ctx, (void*)usrCode);
    startProc(ctx);
    
    //NEED TO FIGURE OUT USERMODE STACK SITCH
    //Should be at 0x800000
    prints("Moving startup package to userland...");
    doffset = 0x100005 + pkgoffset;
    for(i = 0; i < sizes[0]; i++) {
        usrBase[i] = ((char*)doffset)[i];
    }
    prints("Done.\n\n");

    //usermode package should be set up to load at 0x801000
    ctx = newUserProc();
    setProcEntry(ctx, (void*)0x801000);
    startProc(ctx);
    */

    prints("Initializing filesystem\n");    
    fs_init();
    
    //create a ramdisk device from the extents of the kernel payload
    //then install its fs driver and finally mount the ramdisk on root
    doffset = 0x100005 + pkgoffset;
    ram0 = blk_ram_new(doffset, sizes[0]);
    fs_install_driver(get_ramfs_driver());
    fs_attach(FS_RAMFS, ram0, ":");
    
    //This is the final culmination of all our FS and process work
    //start_executable(":startup.bin");
    file_list(":", listBuf);
    prints("File listing of : is '"); prints(listBuf); prints("'");

    while(1);
}


//Start usr prompt
void sys_console() {

    while(1){
        prints(prompt);
        //scans(50, inbuf);
        //parse(inbuf);
        while(1);
    }
}
