#include "../include/p5.h"
#include "../include/registrar.h"
#include "../include/gfx.h"
#include "../include/memory.h"
#include "../include/wyg.h"

typedef struct window {
    unsigned int flags;
    bitmap* context;
    unsigned int child_count;
    window** children;
    
} window;

void main(void) {

    unsigned int parent_pid;

    //Get the 'here's my pid' message from init
    getMessage(&temp_msg);
    parent_pid = temp_msg.source;
    prints("[WYG] Starting WYG GUI services.\n");

    //First thing, register as a WYG service with the registrar
    postMessage(REGISTRAR_PID, REG_REGISTER, SVC_WYG);
    getMessage(&temp_msg);

    if(!temp_msg.payload) {

        prints("\n[WYG] failed to register WYG service.\n");
        postMessage(REGISTRAR_PID, REG_DEREGISTER, SVC_WYG);
        postMessage(parent_pid, 0, 0); //Tell the parent we're done registering
        terminate();
    }

    postMessage(parent_pid, 0, 1); //Tell the parent we're done registering

    //Now we can start the main message loop and begin handling
    //GFX command messages
    while(1) {

        getMessage(&temp_msg);

        switch(temp_msg.command) {

            case WYG_CREATE_WINDOW:
                postMessage(temp_msg.source, GFX_CREATE_WINDOW, (unsigned int)mode_count);
            break;

            default:
            break;
        }
    }
}
