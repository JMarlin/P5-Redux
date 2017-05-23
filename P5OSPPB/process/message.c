#include "process.h"
#include "../memory/memory.h"
#include "../kserver/kserver.h"
#include "message.h"
#include "../ascii_io/ascii_o.h"

#define LOG_MESSAGES 

//Append a message with the given params to the end of the destination
//process's message queue
void passMessage(unsigned int source, unsigned int dest, unsigned int command, unsigned int payload) {

    int i;
    process* d_proc;
    message* new_message;
    message* cur_message;

#ifdef LOG_MESSAGES
    //DEBUG message logging
    if(source != 0 && dest != 0) {
        klog("[Post] From: 0x");
        klogHexDword(source);
        klog(" To: 0x");
        klogHexDword(dest);
        klog(" Command: 0x");
        klogHexDword(command);
        klog(" Payload: 0x");
        klogHexDword(payload);
        klog("\n");
    }
#endif

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
    if(!(new_message = (message*)kmalloc(sizeof(message)))) {
        
        prints("[kernel] kmalloc failed\n");
        return;
    }

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
    if(d_proc->flags & PF_WAITMSG &&
      (d_proc->wait_pid == 0xFFFFFFFF || d_proc->wait_pid == source) &&
      (d_proc->wait_cmd == 0xFFFFFFFF || d_proc->wait_cmd == command)) {
          
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
int getMessage(process* proc, message* msgBuf, unsigned int pid_from, unsigned int command) {

    int i;
    message *prev_msg, *cur_msg;

    //No messages
    if(!proc->root_msg) {

        //DEBUG message logging
#ifdef LOG_MESSAGES        
        if(pid_from != 0) {
            klog("[WaitFor] Proc: 0x");
            klogHexDword(proc->id);
            klog(" Waiting On: 0x");
            klogHexDword(pid_from);
            klog(" Command: 0x");
            klogHexDword(command);
            klog("\n");
        }
#endif

        return 0;
    }

    //Exit early if we're not hunting for a specific message
    if(pid_from == 0xFFFFFFFF && command == 0xFFFFFFFF) {
        
        //Return the matched message
        msgBuf->source = proc->root_msg->source;
        msgBuf->command = proc->root_msg->command;
        msgBuf->payload = proc->root_msg->payload;
        msgBuf->next = (message*)0;

        //DEBUG message logging
#ifdef LOG_MESSAGES
        if(msgBuf->source != 0) {
            klog("[RecvImm] By: 0x");
            klogHexDword(proc->id);
            klog(" From: 0x");
            klogHexDword(msgBuf->source);
            klog(" Command: 0x");
            klogHexDword(msgBuf->command);
            klog(" Payload: 0x");
            klogHexDword(msgBuf->payload);
            klog("\n");
        }
#endif

        //Snip out the matched entry
        proc->root_msg = proc->root_msg->next;

        //Free the memory it was using
        kfree((void*)(proc->root_msg));

        return 1;
    }

    prev_msg = (message*)0;
    cur_msg = proc->root_msg;

    //Look through the queue from the beginning for a match
    while(cur_msg) {

        //Match the source or the command
        if((pid_from == 0xFFFFFFFF || pid_from == cur_msg->source) &&
           (command == 0xFFFFFFFF || command == cur_msg->command)) {

               break;
        }

        prev_msg = cur_msg;
        cur_msg = cur_msg->next;
    }

    //Didn't find any matches!
    if(!cur_msg) {
   
        //DEBUG message logging
#ifdef LOG_MESSAGES        
        if(pid_from != 0) {
            klog("[WaitFor] Proc: 0x");
            klogHexDword(proc->id);
            klog(" Waiting On: 0x");
            klogHexDword(pid_from);
            klog(" Command: 0x");
            klogHexDword(command);
            klog("\n");
        }
#endif

        return 0;
    }

    //Snip out the matched entry
    if(!prev_msg)
        proc->root_msg = cur_msg->next;
    else
        prev_msg->next = cur_msg->next;

    //Return the matched message
    msgBuf->source = cur_msg->source;
    msgBuf->command = cur_msg->command;
    msgBuf->payload = cur_msg->payload;
    msgBuf->next = (message*)0;

    //DEBUG message logging
#ifdef LOG_MESSAGES    
    if(msgBuf->source != 0) {
        klog("[RecvSch] By: 0x");
        klogHexDword(proc->id);
        klog(" From: 0x");
        klogHexDword(msgBuf->source);
        klog(" Command: 0x");
        klogHexDword(msgBuf->command);
        klog(" Payload: 0x");
        klogHexDword(msgBuf->payload);
        klog("\n");
    }
#endif

    //Free the memory it was using
    kfree((void*)(cur_msg));

    return 1;
}

//Look through the process's message queue and return true if there are any messages in it
//which have the specified sender PID and/or command 
int findMessage(struct process* proc, unsigned int send_pid, unsigned int command) {

    message* cur_msg = proc->root_msg;

    while(cur_msg) {

        //Match the source or the command
        if((send_pid == 0xFFFFFFFF || send_pid == cur_msg->source) &&
           (command == 0xFFFFFFFF || command == cur_msg->command)) {

               return 1;
        }

        cur_msg = cur_msg->next;
    }

    return 0;
}