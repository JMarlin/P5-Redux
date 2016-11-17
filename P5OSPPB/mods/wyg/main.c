#include "context.h"
#include "desktop.h"
#include "button.h"
#include "inttypes.h"
#include "../include/p5.h"
#include "../include/registrar.h"
#include "../include/gfx.h"
#include "../include/memory.h"
#include "../include/wyg.h"
#include "../include/key.h"
#include "../include/mouse.h"

//================| Entry Point |================//

short mouse_x;
short mouse_y;
message temp_msg;
unsigned char inbuf[12];

//Our desktop object needs to be sharable by our main function
//as well as our mouse event callback
Desktop* desktop;

//The callback that our mouse device will trigger on mouse updates
void moveMouse(unsigned long packed_data) {

    short x_off, y_off;
    unsigned char buttons;

    buttons = (unsigned char)((packed_data >> 24) & 0xFF);
    x_off = (short)((unsigned char)(packed_data & 0xFF));
    y_off = (short)((unsigned char)((packed_data >> 9) & 0xFF));

    if(packed_data & 0x100)
        mouse_x -= x_off;
    else
        mouse_x += x_off;

    if(packed_data & 0x20000)    
        mouse_y += y_off;
    else
        mouse_y -= y_off;

    if(mouse_x < 0)
        mouse_x = 0;

    if(mouse_x > desktop->window.width - MOUSE_WIDTH)
        mouse_x = desktop->window.width - MOUSE_WIDTH;

    if(mouse_y < 0)
        mouse_y = 0;
    
    if(mouse_y > desktop->window.height - MOUSE_HEIGHT)
        mouse_y = desktop->window.height - MOUSE_HEIGHT;

    Desktop_process_mouse(desktop, (unsigned short)mouse_x, (unsigned short)mouse_y, buttons);
}

void scans(int c, char* b) {

    unsigned char temp_char;
    int index = 0;

    for(index = 0 ; index < c-1 ; ) {
        temp_char = getch();

        if(temp_char != 0) {
            b[index] = temp_char;
            pchar(b[index]);

            if(b[index] == '\n') {
                b[index] = 0;
                break;
            }

            index++;

            if(index == c-1)
                pchar('\n');
        }
    }

    b[index+1] = 0;
}

void showModes(void) {


    unsigned short mode_count;
    unsigned short i;
    screen_mode* mode;

    prints("Enumerating modes...");
    mode_count = enumerateModes();
    prints("done\n");

    prints("\nAvailible modes:\n");
    for(i = 1; i <= mode_count; i++) {

        mode = getModeDetails(i);
        prints("    ");
        printDecimal((unsigned int)i);
        prints(") ");
        printDecimal((unsigned int)mode->width);
        pchar('x');
        printDecimal((unsigned int)mode->height);
        prints(", ");
        printDecimal((unsigned int)mode->depth);
        prints("bpp");

        if(mode->is_linear)
            prints(" linear");

        pchar('\n');
    }
}

//Create and draw a few rectangles and exit
void main(void) {

    unsigned int parent_pid;
    screen_mode* mode;
    unsigned short num;
    unsigned int current_handle;
    int i;
    unsigned int src_pid;
    unsigned char* instr;
    unsigned int strlen;

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

    prints("[WYG] Getting key service...\n");
    if(!initKey()) {
        
        postMessage(REGISTRAR_PID, REG_DEREGISTER, SVC_WYG);
        postMessage(parent_pid, 0, 0); //Tell the parent we're done registering
        terminate();
    }

    prints("[WYG] Getting mouse service...\n");
    if(!initMouse()) {
        
        //Don't need to terminate, but do need to display a warning to the user
    }

    prints("[WYG] Getting graphics service... \n");
    if(!initGfx()) {
        
        //prints("\n[WYG] failed to get the GFX server.\n");
        postMessage(REGISTRAR_PID, REG_DEREGISTER, SVC_WYG);
        postMessage(parent_pid, 0, 0); //Tell the parent we're done registering
        terminate();
    }

    //Prompt user for a screen mode
    showModes();
    prints("mode: ");
    scans(10, inbuf);
    num = inbuf[0] > '9' ? inbuf[0] - 'A' + 10 : inbuf[0] - '0';

    if(!setScreenMode(num)) {

        //prints("[WYG] Could not set screen mode.\n");
        postMessage(REGISTRAR_PID, REG_DEREGISTER, SVC_WYG);
        postMessage(parent_pid, 0, 0); //Tell the parent we're done registering
        terminate();
    }

//    installExceptionHandler((void*)exceptionHandler);

    if(num) {

        mode = getModeDetails(num);
    } else {

        //prints("[WYG] Staying in text mode.\n");
        postMessage(REGISTRAR_PID, REG_DEREGISTER, SVC_WYG);
        postMessage(parent_pid, 0, 0); //Tell the parent we're done registering
        terminate();
    }
    
	//cmd_init(mode->width, mode->height);
	    
    postMessage(parent_pid, 0, 1); //Tell the parent we're done registering

    //Fill this in with the info particular to your project
    Context* context = Context_new(0, 0, 0);
    context->buffer = (unsigned long*)getFramebuffer();
    context->width = mode->width;
    context->height = mode->height;

    //Create the desktop 
    desktop = Desktop_new(context);

    //Set up the initial mouse position
    mouse_x = desktop->window.width / 2 - 1;
    mouse_y = desktop->window.height / 2 - 1;

    //Sprinkle it with windows 
    Window_create_window((Window*)desktop, 10, 10, 300, 200, 0);
    Window* window = Window_create_window((Window*)desktop, 100, 150, 400, 400, 0);
    Window_create_window((Window*)desktop, 200, 100, 200, 600, 0);
    
    //Create and install the button
    Button* button = Button_new(307, 357, 80, 30);
    Window_insert_child(window, (Window*)button);

    Button* button2 = Button_new(307, 267, 80, 30);
    Window_insert_child(window, (Window*)button2);

    //Initial draw
    Window_paint((Window*)desktop, (List*)0, 1);

while(1);

    while(1) {

        //prints("[WYG] Waiting for message...");
        getMessage(&temp_msg);
        //prints("got message ");
         //printDecimal(temp_msg.command);
        ////pchar('\n');

        src_pid = temp_msg.source;

        switch(temp_msg.command) {

            case WYG_CREATE_WINDOW:
			    //cmd_prints("Request to create a new window");
                //postMessage(src_pid, WYG_CREATE_WINDOW, (unsigned int)newWindowHandle((temp_msg.payload & 0xFFF00000) >> 20, (temp_msg.payload & 0xFFF00) >> 8, temp_msg.payload & 0xFF, src_pid));
            break;
            
            case WYG_GET_CONTEXT:
                //postMessage(src_pid, WYG_GET_CONTEXT, (unsigned int)getWindowContext(temp_msg.payload));
            break;
            
            case WYG_GET_DIMS:
                //temp_window = getWindowByHandle(temp_msg.payload);
                //postMessage(src_pid, WYG_GET_DIMS, (unsigned int)((((temp_window->w & 0xFFFF) << 16)) | (temp_window->h & 0xFFFF)));
            break;
            
            case WYG_GET_LOCATION:
                //temp_window = getWindowByHandle(temp_msg.payload);
                //postMessage(src_pid, WYG_GET_LOCATION, (unsigned int)((((temp_window->x & 0xFFFF) << 16)) | (temp_window->y & 0xFFFF)));
            break;
            
            case WYG_MOVE_WINDOW:
                //current_handle = temp_msg.payload;
                //getMessageFrom(&temp_msg, src_pid, WYG_POINT);
                //moveHandle(current_handle, (temp_msg.payload & 0xFFFF0000) >> 16, temp_msg.payload & 0xFFFF);
            break;

            case WYG_INSTALL_WINDOW:
                //current_handle = temp_msg.payload;
                //getMessageFrom(&temp_msg, src_pid, WYG_WHANDLE);
                //installWindow(current_handle, temp_msg.payload);
            break;

            case WYG_SHOW_WINDOW:
                //markHandleVisible(temp_msg.payload, 1);
            break;
            
            case WYG_RAISE_WINDOW:
                //raiseHandle(temp_msg.payload);
            break;

            case WYG_REPAINT_WINDOW:
                //drawHandle(temp_msg.payload);
                //postMessage(src_pid, WYG_REPAINT_WINDOW, 1);
            break;

            case WYG_SET_TITLE:
                //current_handle = temp_msg.payload;
                //postMessage(src_pid, WYG_SET_TITLE, 1);
                //strlen = getStringLength(src_pid);
                //instr = (unsigned char*)malloc(strlen);
                //getString(src_pid, instr, strlen);
                //setWindowTitle(current_handle, instr);
            break;
            
            case WYG_DESTROY:
                //destroyHandle(temp_msg.payload);
                //postMessage(src_pid, WYG_DESTROY, 1);
            break;
            
            case WYG_GET_FRAME_DIMS:
                //postMessage(src_pid, WYG_GET_FRAME_DIMS, (FRAME_SIZE_TOP << 24) | (FRAME_SIZE_LEFT << 16) | (FRAME_SIZE_BOTTOM << 8) | (FRAME_SIZE_RIGHT));
            break;

            case MOUSE_SEND_UPDATE:
                moveMouse(temp_msg.payload);
            break;

            default:
            break;
        }
    }
}
