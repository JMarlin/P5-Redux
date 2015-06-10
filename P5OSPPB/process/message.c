#include "process.h"
#include "../memory/memory.h"
#include "../kserver/kserver.h"
#include "message.h"
#include "../ascii_io/ascii_o.h"

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
        return; //We don't know no process 'round these parts goes by that name, friend

    d_proc = &procTable[i];

    //Make some room for the message
    if(!(new_message = (message*)kmalloc(sizeof(message))))
        return;

    //Insert the data into the newly allocated struct
    new_message->source = source;
    new_message->command = command;
    new_message->payload = payload;
    new_message->next = (message*)0;

    //And then find the end of the queue and put it there
    if(!d_proc->root_msg) {

        d_proc->root_msg = new_message;
    } else {

        cur_message = d_proc->root_msg;

        while(cur_message->next)
            cur_message = cur_message->next;

        cur_message->next = new_message;
    }

    //Finally, if the destination process was sleeping for a message
    //we should tap it on the shoulder and be like 'yo, buddy, wakey
    //wakey eggs and bakey. You've got a buddy at the door'
    if(d_proc->flags & PF_WAITMSG) {

        //Clear wait flag
        d_proc->flags &= ~((unsigned int)PF_WAITMSG);

        //Tell the process manager to pass the message to this dude
        d_proc->flags |= PF_WOKENMSG;
    }
}


//Find the first item in the message queue list of the process with
//id procid, place its contents into the passed message structure
//and finally release the message and shift the list by one
//Returns a 0 if there were no messages
int getMessage(process* proc, message* msgBuf) {

    int i;
    message* next_msg;

    if(!proc->root_msg)
        return 0;

    next_msg = proc->root_msg->next;
    msgBuf->source = proc->root_msg->source;
    msgBuf->command = proc->root_msg->command;
    msgBuf->payload = proc->root_msg->payload;
    msgBuf->next = (message*)0;
    kfree((void*)(proc->root_msg));
    proc->root_msg = next_msg;

    return 1;
}
