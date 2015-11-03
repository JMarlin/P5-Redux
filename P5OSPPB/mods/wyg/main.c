#include "../include/p5.h"
#include "../include/registrar.h"
#include "../include/gfx.h"
#include "../include/memory.h"
#include "../include/wyg.h"

/* Windows are logically arranged as follows:
desktop.first_child:  window_a.first_child:  button_1.first_child:  -
       .next_sibling:         .next_sibling: window_b.first_child:  button_2.first_child:  -
                                                     .next_sibling: -       .next_sibling: button_3.first_child:  -
                                                                                                   .next_sibling: textbox_1.first_child:  -
                                                                                                                           .next_sibling: -
                                                                                                                           
Linkage is in order of increasing z-value                                                                                                
*/

message temp_msg;

typedef struct window {
    unsigned char flags;
    unsigned int handle;
    unsigned int pid;
    bitmap* context;
    struct window* next_sibling;
    struct window* parent; 
    struct window* first_child;
    unsigned int w;
    unsigned int h;
    unsigned int x;
    unsigned int y;
    unsigned char needs_redraw;
} window;

window root_window;
unsigned int next_handle;
unsigned int window_count;
window** registered_windows;

unsigned int newWindow(unsigned int width, unsigned int height, unsigned char flags, unsigned int pid) {
    
    window* new_window;
    unsigned int i, bufsz;
    
    if(!(new_window = (window*)malloc(sizeof(window)))) {
        
        prints("[WYG] Couldn't allocate a new window\n");
        return 0;
    }
    
    //This is currently BAD. If we can't realloc, it destroys the entire engine state in the process.    
    if(!(registered_windows = (window**)realloc((void*)registered_windows, sizeof(window*) * (window_count + 1)))) {
        
        prints("[WYG] Window list realloc failed\n");
        return 0;
    }
    
    prints("[WYG] Created new window, setting initial values\n");
    new_window->pid = pid;
    new_window->flags = flags;
    new_window->next_sibling = (window*)0;
    new_window->parent = (window*)0;
    new_window->first_child = (window*)0;
    new_window->x = 0;
    new_window->y = 0;
    new_window->w = width;
    new_window->h = height;
    
    //Create a drawing context for the new window
    if(!(new_window->context = newBitmap(new_window->w, new_window->h))) {
        
        prints("[WYG] Could not create a new window context\n");
        free((void*)new_window);
        return 0;
    } 
    
    bufsz = new_window->w * new_window->h;
    
    //Clear new window to white
    for(i = 0; i < bufsz; i++)
        new_window->context->data[i] = RGB(255, 255 ,255);
    
    prints("[WYG] Installing new window into window list\n");
    new_window->handle = next_handle++;
    registered_windows[window_count++] = new_window;
    
    prints("[WYG] Successfully created new window ");
    printDecimal(new_window->handle);
    pchar('\n');
    return new_window->handle;
}

window* getWindowByHandle(unsigned int handle) {
    
    int i;
    
    for(i = 0; i < window_count; i++) {
        
        if(registered_windows[i] && registered_windows[i]->handle == handle)
            return registered_windows[i];
    }
    
    return (window*)0;
}

void showModes(void) {


    unsigned short mode_count;
    unsigned short i;
    screen_mode* mode;

    prints("Enumerating modes...");
    mode_count = enumerateModes();
    prints("done\n");

    prints("\nAvailible modes:\n");
    for(i = 0; i < mode_count; i++) {

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

bitmap* getWindowContext(unsigned int handle) {
    
    window* dest_window = getWindowByHandle(handle);
    
    if(!dest_window) {
     
        prints("[WYG] Couldn't find the window to get its context\n");   
        return (bitmap*)0;
    }
        
    return dest_window->context;
}

void moveWindow(unsigned int handle, unsigned short new_x, unsigned short new_y) {
    
    window* dest_window = getWindowByHandle(handle);
    
    if(!dest_window) {
     
        prints("[WYG] Couldn't find the window to set its location\n");   
        return;
    }
        
    dest_window->x = new_x;
    dest_window->y = new_y;
    
    return;
}

void installWindow(unsigned int child_handle, unsigned int parent_handle) {
    
    window* child_window = getWindowByHandle(child_handle);
    window* parent_window = getWindowByHandle(parent_handle);
    window* sibling_window;
    
    if(!child_window || !parent_window) {
     
        prints("[WYG] Couldn't find the parent or child window to perform window install\n");   
        return;
    }
        
    sibling_window = parent_window->first_child;
    
    if(!sibling_window) {
        
        parent_window->first_child = child_window;
        return;
    }
    
    while(sibling_window->next_sibling)
        sibling_window = sibling_window->next_sibling;
        
    sibling_window->next_sibling = child_window;
    
    return;   
}

void markWindowVisible(unsigned int handle) {
    
    window* dest_window = getWindowByHandle(handle);
    
    if(!dest_window) {
     
        prints("[WYG] Couldn't find window to mark it visible\n");   
        return;
    }
        
    dest_window->flags |= WIN_VISIBLE;
    
    return;
}

void markWindowDirty(unsigned int handle) {
    
    window* dest_window = getWindowByHandle(handle);
    
    if(!dest_window) {
     
        prints("[WYG] Couldn't find window to mark it dirty\n");   
        return;
    }
        
    dest_window->needs_redraw = 1;
    
    return;
}

void drawPanel(int x, int y, int width, int height, unsigned int color, int border_width, int invert) {

    unsigned char r = RVAL(color);
    unsigned char g = GVAL(color);
    unsigned char b = BVAL(color);
    unsigned int light_color = RGB(r > 195 ? 255 : r + 60, g > 195 ? 255 : g + 60, b > 195 ? 255 : b + 60);
    unsigned int shade_color = RGB(r < 60 ? 0 : r - 60, g < 60 ? 0 : g - 60, b < 60 ? 0 : b - 60);
    unsigned int temp;
    int i;

    if(invert) {

        temp = shade_color;
        shade_color = light_color;
        light_color = temp;
    }

    for(i = 0; i < border_width; i++) {

        //Top edge
        setCursor(x+i, y+i);
        setColor(light_color);
        drawHLine(width-(2*i));

        //Left edge
        setCursor(x+i, y+i+1);
        drawVLine(height-((i+1)*2));

        //Bottom edge
        setCursor(x+i, (y+height)-(i+1));
        setColor(shade_color);
        drawHLine(width-(2*i));

        //Right edge
        setCursor(x+width-i-1, y+i+1);
        drawVLine(height-((i+1)*2));
    }
}

void drawFrame(window* cur_window) {
    
    prints("[WYG] Drawing frame for window ");
    printDecimal(cur_window->handle);
    pchar('\n');
    
    //Outer border
    drawPanel(cur_window->x - 4, cur_window->y - 28, cur_window->w + 8, cur_window->h + 32, RGB(238, 203, 137), 1, 0);
    
    //Title border
    drawPanel(cur_window->x - 1, cur_window->y - 25, cur_window->w + 2, 22, RGB(238, 203, 137), 1, 1);
    
    //Body border
    drawPanel(cur_window->x - 1, cur_window->y - 1, cur_window->w + 2, cur_window->h + 2, RGB(238, 203, 137), 1, 1);
    
    //Left frame
    setColor(RGB(238, 203, 137));
    setCursor(cur_window->x - 3, cur_window->y - 27);
    fillRect(2, cur_window->h + 30);
    
    //Right frame
    setCursor(cur_window->x + cur_window->w + 1, cur_window->y - 27);
    fillRect(2, cur_window->h + 30);
    
    //Top frame
    setCursor(cur_window->x - 1, cur_window->y - 27);
    fillRect(cur_window->w + 2, 2);
    
    //Mid frame
    setCursor(cur_window->x - 1, cur_window->y - 3);
    fillRect(cur_window->w + 2, 2);
    
    //Bottom frame
    setCursor(cur_window->x - 1, cur_window->y + cur_window->h + 1);
    fillRect(cur_window->w + 2, 2);
    
    //Titlebar
    setColor(RGB(182, 0, 0));
    setCursor(cur_window->x, cur_window->y - 24);
    fillRect(cur_window->w, 20);
    
    //Button
    drawPanel(cur_window->x + cur_window->w - 20, cur_window->y - 24, 20, 20, RGB(238, 203, 137), 1, 0);
    setColor(RGB(238, 203, 137));
    setCursor(cur_window->x + cur_window->w - 19, cur_window->y - 23);
    fillRect(18, 18);
}

//Recursively draw all of the child windows of this window
//At the moment, we're just being super lazy and using 
//painter's algorithm to redraw EVERYTHING
void drawWindow(window* cur_window) {
     
    window* cur_child;
    
    prints("[WYG] Drawing window ");
    printDecimal(cur_window->handle);
    pchar('\n');
    
    if(cur_window->flags & WIN_VISIBLE) {
        
        cur_window->needs_redraw = 0;
        
        //Start by drawing this window
        if(!(cur_window->flags & WIN_UNDECORATED))
            drawFrame(cur_window);
            
        setCursor(cur_window->x, cur_window->y);
        drawBitmap(cur_window->context);
        
        //Then recursively draw all children
        cur_child = cur_window->first_child;
        
        while(cur_child) {
            
            drawWindow(cur_child);
            cur_child = cur_child->next_sibling;
        }
    }
    
    prints("[WYG] Finished drawing window ");
    printDecimal(cur_window->handle);
    pchar('\n');
    
    return;
}

void refreshTree() {
    
    drawWindow(&root_window);
}

void main(void) {

    unsigned int parent_pid;
    screen_mode* mode;
    unsigned short num;
    unsigned int current_handle;
    int i;
    window* temp_window;
    unsigned char inbuf[12];
    unsigned int src_pid;

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

    if(!initGfx()) {
        
        prints("\n[WYG] failed to get the GFX server.\n");
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

        prints("[WYG] Could not set screen mode.\n");
        postMessage(REGISTRAR_PID, REG_DEREGISTER, SVC_WYG);
        postMessage(parent_pid, 0, 0); //Tell the parent we're done registering
        terminate();
    }

    if(num) {

        mode = getModeDetails(num);
    } else {

        prints("[WYG] Staying in text mode.\n");
        postMessage(REGISTRAR_PID, REG_DEREGISTER, SVC_WYG);
        postMessage(parent_pid, 0, 0); //Tell the parent we're done registering
        terminate();
    }
    
    if(!(registered_windows = (window**)malloc(sizeof(window*)))) {
        
        prints("[WYG] Couldn't allocate window LUT.\n");
        postMessage(REGISTRAR_PID, REG_DEREGISTER, SVC_WYG);
        postMessage(parent_pid, 0, 0); //Tell the parent we're done registering
        terminate();
    }
    
    next_handle = 1;
    root_window.handle = next_handle++;
    registered_windows[0] = &root_window;
    window_count = 1;

    //Init the root window (aka the desktop)
    root_window.flags = WIN_UNDECORATED | WIN_FIXEDSIZE | WIN_VISIBLE;
    root_window.next_sibling = (window*)0;
    root_window.parent = (window*)0;
    root_window.first_child = (window*)0;
    root_window.pid = 0;
    root_window.x = 0;
    root_window.y = 0;
    root_window.w = mode->width;
    root_window.h = mode->height;
    
    //Create a drawing context for the root window
    if(!(root_window.context = newBitmap(root_window.w, root_window.h))) {
        
        prints("[WYG] Could not allocate a context for the root window.\n");
        free((void*)registered_windows);
        postMessage(REGISTRAR_PID, REG_DEREGISTER, SVC_WYG);
        postMessage(parent_pid, 0, 0); //Tell the parent we're done registering
        terminate();
    } 

    postMessage(parent_pid, 0, 1); //Tell the parent we're done registering

    //Paint the initial scene
    for(i = 0; i < root_window.w * root_window.h; i++)
        root_window.context->data[i] = RGB(11, 162, 193);
        
    refreshTree();

    //Now we can start the main message loop and begin handling
    //GFX command messages
    while(1) {

        prints("[WYG] Waiting for message...");
        getMessage(&temp_msg);
        prints("got message ");
        printDecimal(temp_msg.command);
        pchar('\n');

        src_pid = temp_msg.source;

        switch(temp_msg.command) {

            case WYG_CREATE_WINDOW:
                postMessage(src_pid, WYG_CREATE_WINDOW, (unsigned int)newWindow((temp_msg.payload & 0xFFF00000) >> 20, (temp_msg.payload & 0xFFF00) >> 8, temp_msg.payload & 0xFF, src_pid));
            break;
            
            case WYG_GET_CONTEXT:
                postMessage(src_pid, WYG_GET_CONTEXT, (unsigned int)getWindowContext(temp_msg.payload));
            break;
            
            case WYG_GET_DIMS:
                temp_window = getWindowByHandle(temp_msg.payload);
                postMessage(src_pid, WYG_GET_DIMS, (unsigned int)((((temp_window->w & 0xFFFF) << 16)) | (temp_window->h & 0xFFFF)));
            break;
            
            case WYG_MOVE_WINDOW:
                current_handle = temp_msg.payload;
                getMessageFrom(&temp_msg, src_pid, WYG_POINT);
                moveWindow(current_handle, (temp_msg.payload & 0xFFFF0000) >> 16, temp_msg.payload & 0xFFFF);
                refreshTree();
            break;

            case WYG_INSTALL_WINDOW:
                current_handle = temp_msg.payload;
                getMessageFrom(&temp_msg, src_pid, WYG_WHANDLE);
                installWindow(current_handle, temp_msg.payload);
            break;

            case WYG_SHOW_WINDOW:
                markWindowVisible(temp_msg.payload);
                refreshTree();
            break;

            case WYG_REPAINT_WINDOW:
                markWindowDirty(temp_msg.payload);
                refreshTree();
            break;

            default:
            break;
        }
    }
}
