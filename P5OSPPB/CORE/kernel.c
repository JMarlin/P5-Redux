#include "../ASCII_IO/ascii_i.h"
#include "../ASCII_IO/ascii_o.h"
#include "cicomp.h"
#include "../FAT12/hiinter.h"
#include "commands.h"
#include "util.h"
#include "../ASCII_IO/keyboard.h"
#include "../memory/memory.h"
#include "../memory/paging.h"

extern long pkgoffset;
extern char imagename;


int main(void)
{
  char prompt[] = "P5-> ";
  char inbuf[50];      
  unsigned int i, doffset, *ksize, *sizes;
  unsigned char *dcount; 

  initScreen();
  setColor(0x1F);
  clear();
  prints("WELCOME TO P5\n");

  if(!enableA20())
  {
        prints("FATAL ERROR: Could not enable the A20 line.\nP5 will now halt.");
        while(1);
  }
  
  prints("Turning on paging...");
  initMMU();
  prints("Paging on.\n");
  init_memory();

  if(!keyboard_init())
        prints("[P5]: No input device availible.\n");

  setupKeyTable();

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

  //Start usr prompt
  while(1)
  {
    pchar('\n');
        prints(prompt);
        scans(50, inbuf);
        parse(inbuf);
  }
  return 0;
}
