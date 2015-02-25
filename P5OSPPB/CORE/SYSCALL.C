#include "syscall.h"
#include "../ascii_io/ascii_o.h"
#include "../ascii_io/ascii_i.h"
#include "../process/process.h"


unsigned int logoMap[] = {
    0b00000000000000000000000000000000,
    0b00000000000000000000000000000000,
    0b00000000000000000000000000000000,
    0b00000000000000000000000000000000,
    0b00001111111000000111111111110000,
    0b00001000000110000100000000000000,
    0b00001000000001000100000000000000,
    0b00001000000000100100000000000000,
    0b00001000000000100100000000000000,
    0b00001000000000100100000000000000,
    0b00001000000000100100000000000000,
    0b00001000000000100100000000000000,
    0b00001000000001000100111110000000,
    0b00001000000110000101000001000000,
    0b00001111111000000110000000100000,
    0b00001000000000000100000000010000,
    0b00001000000000000000000000010000,
    0b00001000000000000000000000010000,
    0b00001000000000000000000000010000,
    0b00001000000000000000000000010000,
    0b00001000000000000000000000010000,
    0b00001000000000000000000000010000,
    0b00001000000000000000000000010000,
    0b00001000000000000000000000010000,
    0b00001000000000000100000000010000,
    0b00001000000000000100000000100000,
    0b00001000000000000010000001000000,
    0b00001000000000000001111110000000,
    0b00000000000000000000000000000000,
    0b00000000000000000000000000000000,
    0b00000000000000000000000000000000,
    0b00000000000000000000000000000000
};
unsigned int lineBuf;

void syscall_exec(void) {

    unsigned char* vram = (unsigned char*)0xA0000;
    unsigned char* buf = (unsigned char*)0x50000;
    int i, j, o, row;

    switch(syscall_number) {

        case 0:
            prints("Program terminated.\n");

			/*THIS IS TEMP*/
            //Clear the screen to white
			o = 0;

            while(1) {
                for(j = 0; j < 200; j++) {

    			    for(i = 0; i < 320; i++) {

    				    buf[j*320 + i] = (j+o) & 0xff;
    				}
    			}


                for(j = 0; j < 32; j++) {

                    row = logoMap[j];
                    for(i = 0; i < 32; i++){
                        if(!(row & 0x80000000)) buf[(j+84)*320 + i + 144] = 0x0f;
                        row = row << 1;
                    }
                }

                o++;

                for(j = 0; j < 320*200; j++)
                    vram[j] = buf[j];
            }

			prints("Done writing to screen.\n");
		    while(1);
        break;

        case 1:
            prints("Printing character\n");
            pchar((unsigned char)(syscall_param1 & 0xFF));
        break;

        case 2:
            p->ctx.ebx = (unsigned int)getch();
        break;

        case 3:
            clear();
        break;
        
        case 4:
            exec_process(":dos.mod");
        break;

        case 5:
            next_process();
        break;
        
        case 6:
            endProc(p);
        break;
                
        default:
        break;
    }
}
