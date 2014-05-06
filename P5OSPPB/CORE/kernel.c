#include "../ASCII_IO/ascii_i.h"
#include "../ASCII_IO/ascii_o.h"
#include "cicomp.h"
#include "../FAT12/hiinter.h"
#include "commands.h"
#include "util.h"
//#include "../ASCII_IO/keyboard.h"
#include "../COFF/COFF.H"

typedef int (*initMethodPtr)(void);

int main(void)
{
  ext_getch = &getch;
  char prompt[] = "P5-> ";
  char inbuf[50];      
  unsigned int i, doffset, *ksize;
  unsigned char *dcount, *module_name;
  initMethodPtr initMethod;

  initScreen();
  setColor(0x1F);
  clear();
  prints("WELCOME TO P5\n");

  if(!enableA20())
  {
        prints("FATAL ERROR: Could not enable the A20 line.\nP5 will now halt.");
        while(1);
  }

  ksize = (unsigned int*)0x1705;
  dcount = (unsigned char*)(ksize[0]+0x1700);

  doffset = 0;      
  for(i = 0; i < dcount[0]; i++){
        module_name = (char*)coff_resolveSymbol("_mname", (void*)(0x1700 + ksize[0] + (dcount[0]*4) + doffset + 1));
        prints("Module #");
        printHexByte((unsigned char)(i&0xFF));
        if(module_name){
                prints(" is '");
                prints(module_name);
                prints("'.\n");
        }else{
                prints(" has no name.\n");
        }

        coff_applyRelocations((void*)(0x1700 + ksize[0] + (dcount[0]*4) + doffset + 1));
        initMethod = (initMethodPtr)coff_resolveSymbol("_modInit",  (void*)(0x1700 + ksize[0] + (dcount[0]*4) + doffset + 1));
        prints("Module started with return code 0x");
        printHexLong(initMethod());
        prints("\n");

        if(strcmpci(module_name, "dellkey")){
                ext_getch = (ext_getchPtr)coff_resolveSymbol("_getch",  (void*)(0x1700 + ksize[0] + (dcount[0]*4) + doffset + 1));
        }

        ext_getch();
        doffset = ((unsigned int*)(0x1700 + ksize[0] + 1 + (4*i)))[0];
  }

  while(1)
  {
    pchar('\n');
        prints(prompt);
        scans(50, inbuf);
        parse(inbuf);
  }
  return 0;
}
