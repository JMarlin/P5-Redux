#include "../process/process.h"
#include "../process/message.h"
#include "../ascii_io/ascii_i.h"
#include "../ascii_io/ascii_o.h"
#include "kserver.h"

void post_to_kern(unsigned int source, unsigned int command, unsigned int payload) {

    int i;
    unsigned char tmp_char;

    switch(command) {
    
        //message 0: terminate
        //look up the process ID for the calling process
        //and unload that process from the system
        case 0:
            
            for(i = 0; i < 256 && (procTable[i].id != source); i++)
            
            if(i == 256)
                return;
                
            endProc(&procTable[i]);
            
            break;
        
        //message 1: pchar
        //Print the character in payload to the screen
        case 1:
            pchar((unsigned int)(payload & 0xFF));
            break;
        
        //message 2: getch
        //Run getch and return a KEY_RECIEVED message to
        //the calling process with the return code in 
        //the payload
        //For now, we'll just return the message right away
        //with a zero value if there's nothing in the chamber
        //for simplicity's sake, but in the future we'll queue
        //this shit up and only send a KEY_RECIEVED message
        //when a keypress is finally registered
        case 2:
            passMessage(0, source, KEY_RECIEVED, (unsigned int)getch());
            break;
        
        //message 3: clear
        //Clear the screen
        case 3:
            clear();
            break;
        
        //message 4: startuserproc
        //Read the string in the userspace at address payload
        //and use it to start a process from the executable at
        //that path, then send a PROC_STARTED message with
        //the ID of the new process in the payload
        case 4:
            passMessage(0, source, PROC_STARTED, exec_process((unsigned char*)payload, 0));
            break;
        
        //message 5: startsuperproc
        //Works the same as the above, but the created process is 
        //a superproc. The new superproc may only be created if the
        //calling proc is also a superproc -- we cannot start superprocs
        //from userprocs because that would defeat the purpose. Once we
        //have users set up we will be able to start a superproc if the
        //owning user is a superuser, but that's down the road
        case 5:
            for(i = 0; i < 256 && (procTable[i].id != source); i++)
            
            if(i == 256)
                return;
            
            if(!(procTable[i].flags & PF_SUPER))
                passMessage(0, source, PROC_STARTED, 0);
            else
                passMessage(0, source, PROC_STARTED, exec_process((unsigned char*)payload, 1));
            
            break;
        
        //Same as exec superproc call but starts a v86 process
        case 6:
            for(i = 0; i < 256 && (procTable[i].id != source); i++)
            
            if(i == 256)
                return;
            
            if(!(procTable[i].flags & PF_SUPER))
                passMessage(0, source, PROC_STARTED, 0);
            else
                passMessage(0, source, PROC_STARTED, exec_v86((unsigned char*)payload));
            
            break;
        
        //Toggle debug: sets or resets the process's debug flag
        case 7:
            for(i = 0; i < 256 && (procTable[i].id != source); i++)
            
            if(i == 256)
                return;
            
            if(procTable[i].flags & PF_DEBUG)
                procTable[i].flags &= ~((unsigned int)PF_DEBUG);
            else
                procTable[i].flags |= PF_DEBUG;
            
            break;
        
        default:
        break;
    }            
}