#include "syscall.h"
#include "../ascii_io/ascii_o.h"
#include "../ascii_io/ascii_i.h"


void syscall_exec(void) {
    
    unsigned char* vram = (unsigned char*)0xA0000;
    int i, j;
    
    switch(syscall_number) {
        
        case 0:
            prints("Program terminated.\n");
	    /*THIS IS TEMP*/
	    for(i = 0; i < 320; i++) {
	    
	        for(j = 0; j < 200; j++) {
		
		    vram[j*320 + i] = 0x0F; 
		}
	    }
	    prints("Done writing to screen.\n");
            while(1);
        break;
        
        case 1:
            prints((unsigned char*)syscall_param1);
        break;
        
        case 2:
            scans(syscall_param1, (unsigned char*)syscall_param2);
        break;
        
        case 3:
            clear();
        break;
        
        default:
        break;
    }
}
