#include "syscall.h"
#include "../process/process.h"
#include "../timer/timer.h"
#include "../process/message.h"

unsigned int gcount = 0;

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

    message temp_message;
    unsigned char* vram = (unsigned char*)0xA0000;
    unsigned char* buf = (unsigned char*)0x50000;
    int i, j, o, row;
        
    switch(syscall_number) {

        //Legacy for V86
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

		    while(1);
        break;

        //Process post message
        case 1:
            passMessage(p->id, p->ctx.ebx, p->ctx.ecx, p->ctx.edx);
        break;

        //Process recieve message
        case 2:
            if(getMessage(p, &temp_message)) {
                p->ctx.eax = 1; //a 1 in eax is 'message found'
                p->ctx.ebx = temp_message->source;
                p->ctx.ecx = temp_message->command;
                p->ctx.edx = temp_message->payload;
            } else {
                p->ctx.eax = 0; //a 0 in eax is 'no messages pending'
                p->ctx.ebx = 0;
                p->ctx.ecx = 0;
                p->ctx.edx = 0;
            }
        break;
        
        default:
        break;
    }
}
