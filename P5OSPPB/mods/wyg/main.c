#include "context.h"
#include "desktop.h"
#include "button.h"
#include "inttypes.h"
#include "wygcommands.h"
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
bitmap* back_buf;
int draw_halt = 0;
int screen_dirty = 0;

unsigned int mouse_count = 0;
unsigned int mouse_queue[100] = {0};

//Our desktop object needs to be sharable by our main function
//as well as our mouse event callback
Desktop* desktop;
extern uint32_t mouse_img[];
uint32_t *fbuf;

void drawStri(char* s, int x, int y) {

    while(*s) {

        setCursor(x, y);
        drawChar(*(s++));
        x += 8;
    }
}

//Do the raw blit of the mouse image onto the screen
void blit_mouse(int mx, int my) {

    int x, y;
    //int mouse_y = desktop->mouse_y;
    //int mouse_x = desktop->mouse_x;

    //No more hacky mouse, instead we're going to rather inefficiently 
    //copy the pixels from our mouse image into the framebuffer
    for(y = 0; y < MOUSE_HEIGHT; y++) {

        //Make sure we don't draw off the bottom of the screen
        if((y + my) >= desktop->window.context->height)
            break;

        for(x = 0; x < MOUSE_WIDTH; x++) {

            //Make sure we don't draw off the right side of the screen
            if((x + mx) >= desktop->window.context->width)
                break;
 
            //Don't place a pixel if it's transparent (still going off of ABGR here,
            //change to suit your palette)
            if(mouse_img[y * MOUSE_WIDTH + x] & 0xFF000000)
                fbuf[(y + my)
                     * desktop->window.context->width 
                     + (x + mx)
                    ] = mouse_img[y * MOUSE_WIDTH + x];
        }
    }
}

void unpackMouse(unsigned long packed_data, short* x_off, short* y_off, unsigned char* buttons) {

    *buttons = (unsigned char)((packed_data >> 24) & 0xFF);
    *x_off = (short)((unsigned char)(packed_data & 0xFF));
    *y_off = (short)((unsigned char)((packed_data >> 9) & 0xFF));

    if(packed_data & 0x100)
        *x_off = -*x_off;

    if(!(packed_data & 0x20000))    
        *y_off = -*y_off;
}

//The callback that our mouse device will trigger on mouse updates
void moveMouse(short x_off, short y_off, unsigned char buttons) {

    //redraw the mouse if it moved
    if(x_off != 0 || y_off != 0) {

        //Wait for the screen thread to be done drawing
        //while(draw_halt);

        //Erase the mouse from its previous location
        //draw_halt++;
        setCursor(mouse_x, mouse_y);
        back_buf->top = mouse_y;
        back_buf->bottom = mouse_y + MOUSE_HEIGHT - 1;
        back_buf->left = mouse_x;
        back_buf->right = mouse_x + MOUSE_WIDTH - 1;
        drawBitmap(back_buf);
        //setCursor(0, 0);
         
        //Reset bitmap draw region
        back_buf->top = back_buf->left = 0;
        back_buf->right = back_buf->width - 1;
        back_buf->bottom = back_buf->height - 1;
        //draw_halt--;
    }

    mouse_x += x_off;   
    mouse_y += y_off;

    if(mouse_x < 0)
        mouse_x = 0;

    if(mouse_x > desktop->window.width - MOUSE_WIDTH)
        mouse_x = desktop->window.width - MOUSE_WIDTH;

    if(mouse_y < 0)
        mouse_y = 0;
    
    if(mouse_y > desktop->window.height - MOUSE_HEIGHT)
        mouse_y = desktop->window.height - MOUSE_HEIGHT;

    //Draw the mouse at the new location
    if(x_off != 0 || y_off != 0) {

        //while(draw_halt);

        //draw_halt++;
        blit_mouse(mouse_x, mouse_y);
        //draw_halt--;
    }

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

void screenThread() {

    while(1) {

        if(screen_dirty && !draw_halt) {
            
            draw_halt++;
            setCursor(0, 0);
            drawBitmap(back_buf);
            blit_mouse(mouse_x, mouse_y);
            screen_dirty = 0;
            draw_halt--;
        }

        sleep(20);
    }
}

void refresh_screen() {

    //Desktop_blit_mouse(desktop);
    //screen_dirty = 1;
    setCursor(0, 0);
    drawBitmap(back_buf);
    blit_mouse(mouse_x, mouse_y);
    //sleep(20);
}

void mdebug_start() {

    prints("[start]\n");
}

void mdebug_end() {

    prints("[end]\n");
    print_malloc_list();
}

//Create and draw a few rectangles and exit
void main(void) {

    unsigned int parent_pid;
    screen_mode* mode;
    unsigned short num;
    unsigned int current_handle;
    unsigned int current_data;
    unsigned int point_data;
    unsigned int color_data;
    unsigned int dim_data;
    int i;
    unsigned int src_pid;
    unsigned char* instr;
    unsigned int strlen;
    unsigned int* test_array = (unsigned int*)0xB01000;
    int last_was_mouse = 0;
    short x_off, y_off;
    unsigned char buttons;

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
    	    
    postMessage(parent_pid, 0, 1); //Tell the parent we're done registering

    //Start malloc debugging
    //enable_debug(mdebug_start, mdebug_end);

    //Create backbuffer
    fbuf = (uint32_t*)getFramebuffer();    
    back_buf = newBitmap(mode->width, mode->height);
    setColor(RGB(255, 0, 0));

    if(!back_buf) {
    
        drawStri("Failed to allocate back_buf", 0, 0);
        while(1);
    }

    //DEBUG print back_buf address
    /*
    for(i = 0; i < 8; i++) {

        unsigned char digit = (((unsigned int)back_buf) >> (4 * (8 - i))) & 0xF;
        setCursor(8*i, 0);

        if(digit < 10)
            drawChar(digit + '0');
        else
            drawChar((digit - 10) + 'A');
    }
    scans(10, inbuf);
    */

    //DEBUG
    /*
    drawStri("About to start the drawing thread...", 0, 0);
    scans(10, inbuf);
    */

    //Would realistically be on a vsync interrupt
    //if(!startThread())
        //screenThread();

    //DEBUG
    /*
    drawStri("About to create the context...", 0, 14);
    scans(10, inbuf);
    */

    //Fill this in with the info particular to your project
    Context* context = Context_new(0, 0, 0);
    context->buffer = (unsigned long*)back_buf->data;
    context->width = mode->width;
    context->height = mode->height;
    Context_set_finalize(context, refresh_screen);

    //DEBUG
    /*
    drawStri("About to create the desktop...", 0, 28);
    scans(10, inbuf);
    */

    //Create the desktop 
    desktop = Desktop_new(context);

    //DEBUG get a pointer to the desktop for crash debugging (see what has a good buffer and what doesn't)
    test_array[0] = (unsigned int)desktop;

    //Set up the initial mouse position
    mouse_x = desktop->window.width / 2 - 1;
    mouse_y = desktop->window.height / 2 - 1;

    //DEBUG
    /*
    drawStri("About to do initial paint...", 0, 42);
    scans(10, inbuf);
    */

    //Do an initial desktop paint
    Window_paint((Window*)desktop, (List*)0, 1);

    //DEBUG
    /*
    drawStri("About to enter message loop...", 0, 56);
    scans(10, inbuf);
    */

    //Main message loop
    while(1) {

        //Flush the mouse queue 
        //We should make this a ring buffer in the future 
        //and allow for the possibility that items in the 
        //mouse queue may cause us to fall back into 'waiting for client mode'
        //and therefore out of this flush loop
        /*
        if(!WYG_waiting_for_clients() && !!mouse_count) {
            int i;

            for(i = 0; i < mouse_count; i++) {

                unpackMouse(mouse_queue[i], &x_off, &y_off, &buttons);
                moveMouse(x_off, y_off, buttons);
            }

            mouse_count = 0;
        }
        */

        getMessage(&temp_msg);

        src_pid = temp_msg.source;

        switch(temp_msg.command) {

            case WYG_CREATE_WINDOW:
                last_was_mouse = 0;
                postMessage(src_pid, WYG_CREATE_WINDOW,
                            WYG_create_window(desktop, temp_msg.payload, src_pid));
            break;
            
            case WYG_GET_CONTEXT:
                last_was_mouse = 0;
                postMessage(src_pid, WYG_GET_CONTEXT,
                            WYG_get_window_context_id(desktop, temp_msg.payload));
            break;
            
            case WYG_GET_DIMS:
                last_was_mouse = 0;
                postMessage(src_pid, WYG_GET_DIMS,
                            WYG_get_window_dimensions(desktop, temp_msg.payload));
            break;
            
            case WYG_GET_LOCATION:
                last_was_mouse = 0;
                postMessage(src_pid, WYG_GET_LOCATION, 
                            WYG_get_window_location(desktop, temp_msg.payload));
            break;
            
            case WYG_MOVE_WINDOW:
                last_was_mouse = 0;
                current_handle = temp_msg.payload;
                getMessageFrom(&temp_msg, src_pid, WYG_POINT);
                WYG_move_window(desktop, current_handle, temp_msg.payload);
            break;

            case WYG_RESIZE_WINDOW:
                last_was_mouse = 0;
                current_handle = temp_msg.payload;
                getMessageFrom(&temp_msg, src_pid, WYG_POINT);
                WYG_resize_window(desktop, current_handle, temp_msg.payload);
            break;

            case WYG_INSTALL_WINDOW:
                last_was_mouse = 0;
                current_handle = temp_msg.payload;
                getMessageFrom(&temp_msg, src_pid, WYG_WHANDLE);
                WYG_install_window(desktop, current_handle, temp_msg.payload);
            break;

            case WYG_SHOW_WINDOW:
                last_was_mouse = 0;
                WYG_show_window(desktop, temp_msg.payload);
            break;
            
            case WYG_RAISE_WINDOW:
                last_was_mouse = 0;
                WYG_raise_window(desktop, temp_msg.payload);
            break;

            case WYG_REPAINT_WINDOW:
                last_was_mouse = 0;
                WYG_invalidate_window(desktop, temp_msg.payload);
                postMessage(src_pid, WYG_REPAINT_WINDOW, 1);
            break;

            case WYG_SET_TITLE:
                last_was_mouse = 0;
                current_handle = temp_msg.payload;
                postMessage(src_pid, WYG_SET_TITLE, 1);
                strlen = getStringLength(src_pid);
                instr = (char*)malloc(strlen);
                getString(src_pid, instr, strlen);
                WYG_set_window_title(desktop, current_handle, instr);
                free((void*)instr);
            break;
            
            case WYG_DESTROY:
                last_was_mouse = 0;
                WYG_destroy_window(desktop, temp_msg.payload);
                postMessage(src_pid, WYG_DESTROY, 1);
            break;
            
            case WYG_GET_FRAME_DIMS:
                last_was_mouse = 0;
                postMessage(src_pid, WYG_GET_FRAME_DIMS,
                            WYG_get_frame_dims());
            break;

            case WYG_DRAW_STRING:
                last_was_mouse = 0;
                //prints("start draw string...");
                current_handle = temp_msg.payload;
                getMessageFrom(&temp_msg, src_pid, WYG_POINT);
                //prints("got string position...");
                current_data = temp_msg.payload;
                postMessage(src_pid, WYG_DRAW_STRING, 1);
                //prints("sent confirm message...");
                strlen = getStringLength(src_pid);
                //prints("got string length...");
                instr = (char*)malloc(strlen); //Probably needs to be strlen +1? Could be overwriting the head of the next malloc block
                //prints("allocated string buffer...");
                getString(src_pid, instr, strlen); 
                //prints("received string data...");
                WYG_draw_string(desktop, current_handle, current_data, instr); //String freed after draw
                //prints("drew string to screen...");
                //prints("done");
            break;

            case WYG_DRAW_RECT:
                last_was_mouse = 0;
                current_handle = temp_msg.payload;
                getMessageFrom(&temp_msg, src_pid, WYG_POINT);
                point_data = temp_msg.payload;
                getMessageFrom(&temp_msg, src_pid, WYG_DIMS);
                dim_data = temp_msg.payload;
                getMessageFrom(&temp_msg, src_pid, WYG_COLOR);
                WYG_draw_rectangle(desktop, current_handle, point_data, dim_data, temp_msg.payload);
                postMessage(src_pid, WYG_DRAW_RECT, 1);
            break;

            case WYG_PAINT_DONE:
                last_was_mouse = 0;
                WYG_finish_window_draw(desktop, temp_msg.payload);
            break;

            case MOUSE_SEND_UPDATE:
                /*
                if(last_was_mouse && WYG_waiting_for_clients()) {

                    //If the queue fills up, we just start throwing things into space
                    if(mouse_count < 100)
                        mouse_queue[mouse_count++] = temp_msg.payload;
                } else {

                    last_was_mouse = 1;

                    if(mouse_count)
                        unpackMouse(mouse_queue[mouse_count--], &x_off, &y_off, &buttons);
                    else 
                        unpackMouse(temp_msg.payload, &x_off, &y_off, &buttons);

                    moveMouse(x_off, y_off, buttons);
                }
                */

                //testing THIS IS COMPLETELY AND OBVIOUSLY WRONG
                //if(!WYG_waiting_for_clients()) {

                    unpackMouse(temp_msg.payload, &x_off, &y_off, &buttons);

                    moveMouse(x_off, y_off, buttons);
                //}
            break;

            default:
            break;
        }
    }

}
