#include "syscall.h"
#include "../process/process.h"
#include "../timer/timer.h"
#include "../process/message.h"
#include "../ascii_io/ascii_o.h"

unsigned int gcount = 0;
unsigned int lineBuf;
void (*call_zero_callback)(unsigned int, unsigned int, unsigned int);

void set_call_zero_cb(void (*cb)(unsigned int, unsigned int, unsigned int)) {

    call_zero_callback = cb;
}


void syscall_exec(void) {

    message temp_message;
    unsigned int wait_pid;
    unsigned int wait_cmd;
    unsigned int tmp_ebx;
    unsigned int tmp_ecx;
    unsigned int tmp_edx;

    switch(_syscall_number) {

        //Return to kernel
        //used for early start-up kernel processes to execute
        //and then return control to the section of kernel which
        //started it. One major use case is v86 helper code, as in
        //the memory system. We need to map the memory before the
        //system is booted, but the only way to use the bios
        //memory maps commands is to start a v86 process. So the
        //memory initialization code writes some temp code to low
        //memory, installs its callback function to be called when
        //the process exits using syscall 0 and continues the
        //execution from there
        case 0:
            tmp_ebx = p->ctx.ebx;
            tmp_ecx = p->ctx.ecx;
            tmp_edx = p->ctx.edx;
            deleteProc(p); //We do this instead of endProc because endProc
                           //attempts to reenter the scheduler and we never
                           //flow through to the next line
            call_zero_callback(tmp_ebx, tmp_ecx, tmp_edx);
        break;

        //Process post message
        case 1:            
            passMessage(p->id, p->ctx.ebx, p->ctx.ecx, p->ctx.edx);
        break;

        //Process recieve message
        case 2:
            //If there is a message in the queue, send it
            //otherwise, put the process to sleep
            if((p->ctx.ebx & 0xFFFF) == 0xFFFF) {
                wait_pid = p->ctx.ecx;
                wait_cmd = p->ctx.edx;
            } else {
                wait_pid = wait_cmd = 0xFFFFFFFF;
            }

            if(getMessage(p, &temp_message, wait_pid, wait_cmd)) {

                p->ctx.eax = 1; //a 1 in eax is 'message found'
                p->ctx.ebx = temp_message.source;
                p->ctx.ecx = temp_message.command;
                p->ctx.edx = temp_message.payload;
            } else {

                p->flags |= PF_WAITMSG;
                p->wait_pid = wait_pid;
                p->wait_cmd = wait_cmd;
            }
        break;

        //Process look for message
        case 3:
            p->ctx.eax = findMessage(p, p->ctx.ecx, p->ctx.edx);
        break;

        default:
        break;
    }
}
