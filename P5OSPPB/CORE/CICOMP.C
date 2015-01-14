#include "cicomp.h"
#include "commands.h"
#include "../ASCII_IO/ascii_o.h"

void parse(char* instr)
{     
     
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
                  prints("PiCI Error: PEBKAC\n");        
              }
           }
        }
     }
  }
}
