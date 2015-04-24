#include "cicomp.h"
#include "commands.h"
#include "../ascii_io/ascii_o.h"


void foo(void) {

    int i;

    prints("ESP: ");
    __asm__ ("\t movl %%esp, %0" : "=r"(i));
    printHexDword(i);
    prints("\n");
    __asm__ ("jmp foo");
}

void parse(char* instr) {

  int i;

    if(findfatfile(instr) && ( slen(instr) <= 8 )) {

        if(strcmpci("SPB",ext(instr))) {
            fatexec(instr);
        } else {
            prints("PiCI Error: Not an SPB executable\n");
        }
    } else {

        if(strcmpci(instr,"CLR")) {
            clear();
        } else {

            if(strcmpci(instr,"CHPROMPT")) {
               chprompt();
            } else {

                if(strcmpci(instr,"HELP")) {
                    help();
                } else {

                    if(strcmpci(instr,"TSTVAR")) {
                        testvars();
                    } else {

                        if(strcmpci(instr, "INT")) {
                            __asm__ ("int $0x80 \n");
                        } else {
                            prints("PiCI Error: PEBKAC\n");
                        }
                    }
                }
            }
        }
    }
}
