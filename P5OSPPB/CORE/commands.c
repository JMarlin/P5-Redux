#include "commands.h"
#include "../ASCII_IO/ascii_o.h"

void clear(void)
{
  int i;
  setCursor(0, 0);
  for(i = 0; i < 2000; i++)
        pchar(0);
  setCursor(0, 0);
}

void chprompt(void)
{
   prints("Sorry, not yet implemented.");        
}

void help(void)
{
   clear();
   
   prints("Protical5 PinkCI command interpreter               ");pchar(10);
   prints("version R0                                         ");pchar(10);
   prints("Written by Joseph Marlin (stithyoshi@sbcglobal.net)");pchar(10);
   prints("---------------------------------------------------");pchar(10);
   prints("   Commands are:                                   ");pchar(10);
   prints("                                                   ");pchar(10);
   prints("        CLR..................Clear the screen.     ");pchar(10);
   prints("        CHPROMPT.............Not yet implemented.  ");pchar(10);
   prints("        HELP.................Print this message.   ");pchar(10);
   prints("                                                   ");pchar(10);
           
}
