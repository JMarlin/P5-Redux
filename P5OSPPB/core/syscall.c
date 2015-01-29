#include "syscall.h"
#include "../ascii_io/ascii_o.h"
#include "../ascii_io/ascii_i.h"


void syscall_exec(void) {
    
    switch(syscall_number) {
        
        case 0:
            prints("Program terminated.\n");
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