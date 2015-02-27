#include "process.h"
#include "../memory/memory.h"
#include "../kserver/kserver.h"

//Append a message with the given params to the end of the destination
//process's message queue
void passMessage(unsigned int source, unsigned int dest, unsigned int command, unsigned int payload) {

    int i;
    process* d_proc;
    message* new_message;
    message* cur_message;
    
    //In the messaging scheme, process zero is the kernel
    //This will be used for starting new processes and other
    //core OS services
    if(!dest) {
        
        post_to_kern(source, command, payload);
        return;
    }
        
    //Otherwise, we'll look up the destination process and
    //append the new message to its queue
    for(i = 0; i < 256 && procTable[i].id != dest; i++);
    
    if(i == 256)
        return;
    
    d_proc = &procTable[i];
    
    if(!(new_message = (message*)kmalloc(sizeof(message))))
        return;
    
    new_message->source = source;
    new_message->command = command;
    new_message->payload = payload;
    new_message->next = (message*)0;
    
    if(!d_proc->root_msg) {
    
        d_proc->root_msg = new_message;
        return;
    }
    
    cur_message = d_proc->root_msg;
    
    while(cur_message->next)
        cur_message = cur_message->next;
        
    cur_message->next = new_message;
}


//Find the last item in the message queue list of the process with
//id procid, place its contents into the passed message structure
//and finally release the message and truncate the list
//Returns a 0 if there were no messages
int getMessage(process* proc, message* msgBuf) {

    int i;
    message* cur_msg;
    message* prev_msg = (message*)0;
       
    if(!proc->root_msg)
        return 0;
        
    cur_msg = proc->root_msg;
    
    while(cur_msg->next) {
    
        prev_msg = cur_msg;
        cur_msg = cur_msg->next;
    }
    
    msgBuf->source = cur_msg->source;
    msgBuf->command = cur_msg->command;
    msgBuf->payload = cur_msg->payload;
    msgBuf->next = (message*)0;
    kfree((void*)cur_msg);
    
    if(prev_msg) {
        
        prev_msg->next = (message*)0;
    } else {
    
        proc->root_msg = (message*)0;
    }
    
    return 1;
}
