#include "cicomp.h"
#include "commands.h"
#include "../ASCII_IO/ascii_o.h"

void foo(void) {
    int i;
    prints("ESP: ");
    __asm__ ("\t movl %%esp, %0" : "=r"(i));
    printHexDword(i);
    prints("\n");
    __asm__ ("jmp _foo");
}

void parse(char* instr)
{     

  int i;
     
  if(findfatfile(instr) && ( slen(instr) <= 8 ))
  {        
     if(strcmpci("SPB",ext(instr)))
     {
        fatexec(instr);        
     }else{     
        prints("PiCI Error: Not an SPB executable\n");             
     }
  }else{
     if(strcmpci(instr,"CLR"))
     {
        clear();        
     }else{
        if(strcmpci(instr,"CHPROMPT"))
        {
           chprompt();        
        }else{
           if(strcmpci(instr,"HELP"))
           {
              help();           
           }else{
              if(strcmpci(instr,"TSTVAR"))
              {
                  testvars();
              }else{
                if(strcmpci(instr, "REC")) {
                  foo();
                } else {
                  prints("PiCI Error: PEBKAC\n");        
                }
              }
           }
        }
     }
  }
}
