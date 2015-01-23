#include "../process/process.h"
#include "kernel.h"
#include "../ASCII_IO/ascii_i.h"
#include "../ASCII_IO/ascii_o.h"
#include "cicomp.h"
#include "../FAT12/hiinter.h"
#include "commands.h"
#include "util.h"
#include "int.h"
#include "../ASCII_IO/keyboard.h"
#include "../memory/memory.h"
#include "../memory/paging.h"
#include "../memory/gdt.h"

extern long pkgoffset;
extern char imagename;

char prompt[] = "P5-> ";
char inbuf[50];   

int main(void)
{   
  unsigned int i, doffset, *ksize, *sizes;
  unsigned char *dcount; 

  initScreen();
  setColor(0x1F);
  clear();

/*
  if(!enableA20())
  {
        prints("FATAL ERROR: Could not enable the A20 line.\nP5 will now halt.");
        while(1);
  }
*/

  prints("Setting up the GDT...");
  initGdt();
  prints("done.");

  prints("Setting up interrupt table...");
  initIDT();
  prints("done.\n");
  installExceptionHandlers();

  prints("Turning on paging...");
  initMMU();
  prints("Paging on.\n");
  //init_memory(); Need to overhaul the kmalloc system based on paging
  
  if(!keyboard_init())
        prints("[P5]: No input device availible.\n");

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

  sys_console();

  return 0;
}

//Start usr prompt
void sys_console() {
    while(1)
    {
        pchar('\n');
        prints(prompt);
        scans(50, inbuf);
        parse(inbuf);
    }
}

