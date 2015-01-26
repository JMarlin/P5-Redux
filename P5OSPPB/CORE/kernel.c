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


extern long pkgoffset;
extern char imagename;
char prompt[] = "P5-> ";
char inbuf[50];   


int main(void) {   

    unsigned int i, doffset, *ksize, *sizes;
    unsigned char *dcount; 

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

    prints("Setting up the GDT...");
    initGdt();
    prints("done.\nSetting up interrupt table...");
    initIDT();
    installExceptionHandlers();
    prints("done.\nTurning on paging...");
    initMMU();
    prints("Done.\nSetting up keyboard...");
  
    if(!keyboard_init())
        prints("Failed\n[P5]: No input device availible.\n");
    else
        prints("Done.\n");

    setupKeyTable();
    prints("WELCOME TO P5\n");
    ksize = (unsigned int*)0x100005;

/*  dcount = (unsigned char*)(0x1700+ksize[0]);
  sizes = (unsigned int*)(0x100001+ksize[0]);
  if(!ksize || (ksize && !dcount)){
        prints("No modules found.\n");
  }else{

        doffset = 4 + (dcount[0]*4);      
        for(i = 0; i < dcount[0]; i++){
                prints("0x");
                printHexDword(sizes[i]); 
                prints(" byte module found at image+0x");
                printHexDword(doffset); 
                pchar('\n'); 
                doffset += sizes[i];
        }
  }  
*/
  
    //Print kernel size and version
    prints("Image: ");
    prints(&imagename);
    prints("\nSize: ");
    printHexDword(pkgoffset);
    prints("b\n"); 
    prints("ESP: ");
    asm("\t movl %%esp, %0" : "=r"(i));
    printHexDword(i);
    pchar('\n\n');
    sys_console();
    return 0;
}


//Start usr prompt
void sys_console() {
    
    while(1){
        prints(prompt);
        scans(50, inbuf);
        parse(inbuf);
    }
}
