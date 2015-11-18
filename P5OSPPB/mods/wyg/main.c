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

#define new(x) (((x)*)malloc(sizeof(x)))

message temp_msg;

typedef struct rect {
    unsigned int top;
    unsigned int right;
    unsigned int bottom;
    unsigned int left;
} rect;

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
    unsigned char* title;
    unsigned char frame_needs_redraw;
} window;

window root_window;
unsigned int next_handle;
unsigned int window_count;
window** registered_windows;
unsigned char inited = 0;

/*!!!!!!!!!! DEBUG SHIT !!!!!!!!!
unsigned char cmd_x;
unsigned char cmd_y;
int cmd_width;
int cmd_height;
int cmd_max_chars, cmd_max_lines;

void drawCharacter(char c, int x, int y, unsigned int color) {

    setCursor(x, y);
    setColor(color);
    drawChar(c);
}

void cmd_getCursor(unsigned char *x, unsigned char *y) {

    if(!inited)
        return;

    *x = cmd_x;
    *y = cmd_y;
}

void cmd_putCursor(unsigned char x, unsigned char y) {

    if(!inited)
        return;

    cmd_x = x;
    cmd_y = y;
}

void cmd_clear() {

    if(!inited)
        return;

    setCursor(0, 0);
    setColor(RGB(255, 255, 255));
    fillRect(cmd_width, cmd_height);
    cmd_x = 0;
    cmd_y = 0;
}

void cmd_pchar(unsigned char c) {

    if(!inited)
        return;

    if(c == '\n') {

        cmd_x = 0;
        cmd_y++;
        
        if(cmd_y > cmd_max_lines) 
            cmd_clear();
    } else {

        drawCharacter(c, (cmd_x*8), (cmd_y*12), RGB(0, 0, 0));
        cmd_x++;

        if(cmd_x > cmd_max_chars) {

            cmd_x = 0;
            cmd_y++;
            
            if(cmd_y > cmd_max_lines) 
                cmd_clear();
        }
    }
}

void cmd_prints(unsigned char* s) {

    if(!inited)
        return;

    while(*s)
        cmd_pchar(*s++);
}

void cmd_printClear(int count) {

    if(!inited)
        return;

    setCursor((cmd_x*8), (cmd_y*12));
    setColor(0);
    fillRect(8*count, 12);
    cmd_x += count;
}

void cmd_printDecimal(unsigned int dword) {

    if(!inited)
        return;

    unsigned char digit[12];
    int i, j;

    i = 0;
    while(1) {

        if(!dword) {

            if(i == 0)
                digit[i++] = 0;

            break;
        }

        digit[i++] = dword % 10;
        dword /= 10;
    }

    for(j = i - 1; j >= 0; j--)
        cmd_pchar(digit[j] + '0');
}

void cmd_printHexByte(unsigned char byte) {

    if(!inited)
        return;

    cmd_pchar(digitToHex((byte & 0xF0)>>4));
    cmd_pchar(digitToHex(byte & 0xF));
}


void cmd_printHexWord(unsigned short wd) {

    if(!inited)
        return;

    cmd_printHexByte((unsigned char)((wd & 0xFF00)>>8));
    cmd_printHexByte((unsigned char)(wd & 0xFF));
}


void cmd_printHexDword(unsigned int dword) {

    if(!inited)
        return;

    cmd_printHexWord((unsigned short)((dword & 0xFFFF0000)>>16));
    cmd_printHexWord((unsigned short)(dword & 0xFFFF));
}

void cmd_init(unsigned short xres, unsigned short yres) {

    cmd_x = 0;
    cmd_y = 0;
    cmd_width = xres;
    cmd_height = yres;
    cmd_max_chars = (cmd_width/8) - 1;
    cmd_max_lines = (cmd_height/12) - 1;
    inited = 1;
}
!!!!!!!!!! DEBUG SHIT !!!!!!!!!*/

void drawWindow(window* dest_window);
void raiseWindow(window* dest_window);

unsigned int newWindow(unsigned int width, unsigned int height, unsigned char flags, unsigned int pid) {
    
    window* new_window;
    unsigned int i, bufsz;
    
    if(!(new_window = (window*)malloc(sizeof(window)))) {
        
        //cmd_prints("[WYG] Couldn't allocate a new window\n");
        return 0;
    }
    
    //This is currently BAD. If we can't realloc, it destroys the entire engine state in the process.    
    if(!(registered_windows = (window**)realloc((void*)registered_windows, sizeof(window*) * (window_count + 1)))) {
        
        //cmd_prints("[WYG] Window list realloc failed\n");
        return 0;
    }
    
    //cmd_prints("[WYG] Created new window, setting initial values\n");
    new_window->pid = pid;
    new_window->flags = flags;
    new_window->next_sibling = (window*)0;
    new_window->parent = (window*)0;
    new_window->first_child = (window*)0;
    new_window->x = 0;
    new_window->y = 0;
    new_window->w = width;
    new_window->h = height;
    new_window->title = (unsigned char*)0;
    new_window->frame_needs_redraw = 1;
    
    //Create a drawing context for the new window
    if(!(new_window->context = newBitmap(new_window->w, new_window->h))) {
        
        //cmd_prints("[WYG] Could not create a new window context\n");
        free((void*)new_window);
        return 0;
    } 
    
    bufsz = new_window->w * new_window->h;
    
    //Clear new window to white
    for(i = 0; i < bufsz; i++)
        new_window->context->data[i] = RGB(255, 255 ,255);
    
    //cmd_prints("[WYG] Installing new window into window list\n");
    new_window->handle = next_handle++;
    registered_windows[window_count++] = new_window;
    
    //cmd_prints("[WYG] Successfully created new window ");
    //cmd_printDecimal(new_window->handle);
    //cmd_pchar('\n');
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
     
        //cmd_prints("[WYG] Couldn't find the window to get its context\n");   
        return (bitmap*)0;
    }
        
    return dest_window->context;
}

//Redraws every window intersected by window_bounds
void updateOverlapped(rect* window_bounds) {
    
    int i;
    rect comp_rect; 
        
    for(i = 0; i < window_count; i++) {
        
        comp_rect.top = registered_windows[i]->y;
        comp_rect.left = registered_windows[i]->x;
        comp_rect.bottom = comp_rect.top + registered_windows[i]->h - 1;
        comp_rect.right = comp_rect.left + registered_windows[i]->w - 1;
        
        if(window_bounds->left <= comp_rect.right &&
           window_bounds->right >= comp_rect.left &&
           window_bounds->top <= comp_rect.bottom && 
           window_bounds->bottom >= comp_rect.top)
            drawWindow(registered_windows[i]);
    }
}

void moveWindow(unsigned int handle, unsigned short new_x, unsigned short new_y) {
    
    window* dest_window = getWindowByHandle(handle);
    rect overlap_rect;
    
    if(!dest_window) {
     
        //cmd_prints("[WYG] Couldn't find the window to set its location\n");   
        return;
    }
    
    //If a window is moved, we must ensure that it is the active window 
    raiseWindow(dest_window);
    
    //Create a rectangle covering the old location for later intersection
    overlap_rect.top = dest_window->y;
    overlap_rect.left = dest_window->x;
    overlap_rect.bottom = overlap_rect.top + dest_window->h - 1;
    overlap_rect.right = overlap_rect.left + dest_window->w - 1;
        
    dest_window->x = new_x;
    dest_window->y = new_y;
    
    //Need to update the screen if we're visible    
    if(dest_window->flags & WIN_VISIBLE) {
                
        updateOverlapped(&overlap_rect); //Redraw all of the siblings that this window was covering up
        
        //Redraw the window at its new location
        dest_window->frame_needs_redraw = 1;
        drawWindow(dest_window);
    } 
        
    return;
}

void installWindow(unsigned int child_handle, unsigned int parent_handle) {
    
    window* child_window = getWindowByHandle(child_handle);
    window* parent_window = getWindowByHandle(parent_handle);
    window* sibling_window;
    
    if(!child_window || !parent_window) {
     
        //cmd_prints("[WYG] Couldn't find the parent or child window to perform window install\n");   
        return;
    }
    
    child_window->parent = parent_window;    
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

void markWindowVisible(window* dest_window, unsigned char is_visible) {
    
    unsigned char was_visible;
    rect overlap_rect;
    
    was_visible = dest_window->flags & WIN_VISIBLE;

    if(is_visible)        
        dest_window->flags |= WIN_VISIBLE;
    else
        dest_window->flags &= ~((unsigned char)WIN_VISIBLE);
    
    if(was_visible & !is_visible) {
        
        overlap_rect.top = dest_window->y;
        overlap_rect.left = dest_window->x;
        overlap_rect.bottom = overlap_rect.top + dest_window->h - 1;
        overlap_rect.right = overlap_rect.left + dest_window->w - 1;
        
        updateOverlapped(&overlap_rect); //Redraw all of the siblings that this window was covering up
    } 
    
    return;
}

void markHandleVisible(unsigned int handle, unsigned char is_visible) {
    
    window* dest_window = getWindowByHandle(handle);
    
    if(!dest_window) {
     
        //cmd_prints("[WYG] Couldn't find window to mark it visible\n");   
        return;
    }
    
    markWindowVisible(dest_window, is_visible);
}

void markWindowDirty(unsigned int handle) {
    
    window* dest_window = getWindowByHandle(handle);
    
    if(!dest_window) {
     
        //cmd_prints("[WYG] Couldn't find window to mark it dirty\n");   
        return;
    }
        
    dest_window->needs_redraw = 1;
    
    return;
}

void setWindowTitle(unsigned int handle, unsigned char* newstr) {
    
    window* dest_window = getWindowByHandle(handle);
    
    if(!dest_window) {
     
        //cmd_prints("[WYG] Couldn't find window to mark it dirty\n");   
        return;
    }
    
    if(dest_window->title)
        free(dest_window->title);
        
    dest_window->title = newstr;
    
    dest_window->frame_needs_redraw = 1;
}

void drawPanel(int x, int y, int width, int height, unsigned int color, int border_width, int invert) {

    unsigned char r = RVAL(color);
    unsigned char g = GVAL(color);
    unsigned char b = BVAL(color);
    unsigned int light_color = RGB(r > 155 ? 255 : r + 100, g > 155 ? 255 : g + 100, b > 155 ? 255 : b + 100);
    unsigned int shade_color = RGB(r < 100 ? 0 : r - 100, g < 100 ? 0 : g - 100, b < 100 ? 0 : b - 100);
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

void drawTitlebar(window* cur_window, unsigned char active) {
    
    unsigned char* s;    
    
    //Titlebar
    if(active)
        setColor(RGB(182, 0, 0));
    else 
        setColor(RGB(238, 203, 137));
    
    setCursor(cur_window->x, cur_window->y - 24);
    fillRect(cur_window->w - 20, 20);
    
     //Window title
    if(cur_window->title) {
        
        //cmd_prints(cur_window->title);
        
        int base_x, base_y, off_x, titlebar_width;
        
        s = cur_window->title;
        base_x = cur_window->x + 2;
        base_y = cur_window->y - 20;
        off_x = 0;
        titlebar_width = cur_window->w - 20;
        
        if(active)
            setColor(RGB(255, 255, 255));
        else
            setColor(RGB(138, 103, 37));
        
        while(*s) {
            
            setCursor(base_x + off_x, base_y);
            drawChar(*(s++));
            off_x += 8;
            
            //Truncate the text if it's wider than the titlebar
            if(off_x >= titlebar_width)
                break;
        }
    }
}

void drawFrame(window* cur_window) {
    
    int i;
    
    //cmd_prints("[WYG] Drawing frame for window ");
    //cmd_printDecimal(cur_window->handle);
    //cmd_pchar('\n');
    
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
        
    //Button
    drawPanel(cur_window->x + cur_window->w - 20, cur_window->y - 24, 20, 20, RGB(238, 203, 137), 1, 0);
    setColor(RGB(238, 203, 137));
    setCursor(cur_window->x + cur_window->w - 19, cur_window->y - 23);
    fillRect(18, 18);
    
    drawTitlebar(cur_window, cur_window->next_sibling == (window*)0);
    
    cur_window->frame_needs_redraw = 0;
}

//   Developing a proper redraw routine is going to be one of
//the most key aspects of getting a responsive window manager.
//One of the current major issues we're experiencing thus far
//is that small, repeated redraw requests are filling up the 
//message buffer and slowing everything to a crawl (this is 
//currently happening on our terminal which is redrawing with
//each printed character)
//   The obvious answer is twofold: Firstly, we need to prevent
//a window from requesting another redraw until the current 
//redraw has completed. Secondly, we need to allow for
//specifying the region of the redraw within the window bitmap
//to minimize the amount of time that the redraw call spends
//blitting to the screen. For this, we can use the bitmap's
//built-in blitting rectangle.
//   The final component is to allow for the redrawing of 
//background windows that may be semi-occluded by overlapping
//siblings. This will be done by using a recursive splitting 
//algorithm to break down the non-occluded rectilinear
//polygon defining the visible area of the window into 
//rectangles, each of which will then be sent to the GFX 
//bitmap blitter to be inserted into the framebuffer -- this
//will require finally implementing the blit rectangle in 
//GFX as well. In this phase we'll be able to do things to
//significantly improve performance such as throwing out a
//window for redraw the moment it is determined to be 
//completely occluded and generalizing that to the input 
//blitting rectangle as well. EG: If the region we want
//redrawn is under another window, don't bother drawing it.

void drawWindow(window* cur_window) {
     
    window* cur_child;
    
    //cmd_prints("[WYG] Drawing window ");
    //cmd_printDecimal(cur_window->handle);
    //cmd_pchar('\n');
    
    if(cur_window->flags & WIN_VISIBLE) {
        
        cur_window->needs_redraw = 0;
        
        //Start by drawing this window
        if(!(cur_window->flags & WIN_UNDECORATED) && cur_window->frame_needs_redraw)
            drawFrame(cur_window);
        
        //For now, we'll just pass the given blit mask on to
        //the GFX system unmodified because we don't care
        //about occlusion yet. But note that this is where
        //we would extract that info into a temp rectangle 
        //upon which we will do our occlusion splitting/testing    
        setCursor(cur_window->x, cur_window->y);
        drawBitmap(cur_window->context);
               
        /*
        //Then recursively draw all children
        cur_child = cur_window->first_child;
        
        while(cur_child) {
            
            drawWindow(cur_child);
            cur_child = cur_child->next_sibling;
        }
        */
    }
    
    //cmd_prints("[WYG] Finished drawing window ");
    //cmd_printDecimal(cur_window->handle);
    //cmd_pchar('\n');
    
    return;
}

void drawHandle(unsigned int handle) {
    
    window* dest_window = getWindowByHandle(handle);
    
    if(!dest_window) {
     
        //cmd_prints("[WYG] Couldn't find the window to be raised\n");   
        return;
    }
    
    drawWindow(dest_window);
}

void drawDeactivated(window* owner) {
    
    window* cur_sibling;
    
    //Find the active child of the owner
    cur_sibling = owner->first_child;
    
    //No children, we can quit
    if(!cur_sibling)
        return;
        
    while(cur_sibling->next_sibling)
        cur_sibling = cur_sibling->next_sibling;
        
    //cur_sibling is now the active sibling and can have its titlebar redrawn
    if(cur_sibling->flags & WIN_VISIBLE && !(cur_sibling->flags & WIN_UNDECORATED))
        drawTitlebar(cur_sibling, 0);
    
    //Iterate down the active branch until we're out of children
    drawDeactivated(cur_sibling);
}

void raiseWindow(window* dest_window) {
    
    window* parent;
    window* owning_sibling;
    window* old_active;
        
    //If the window isn't visible, it will need to be in order to be raised 
    dest_window->flags |= WIN_VISIBLE;
    
    //Can't raise the root window
    if(dest_window->handle == 1)
        return ;
        
    //We don't need to do anything if the window is parentless or already at the end of its chain
    if(!dest_window->parent || !dest_window->next_sibling)
        return;
    
    //Deactivate the titlebars of all previously active siblings
    drawDeactivated(dest_window->parent);
    
    //Set up the window iterators    
    parent = dest_window->parent;
    owning_sibling = parent->first_child;
    
    //This only happens if the dest window is the bottommost child
    if(owning_sibling == dest_window) {
        
        //Find the end window
        while(owning_sibling->next_sibling)
            owning_sibling = owning_sibling->next_sibling;
        
        parent->first_child = dest_window->next_sibling;
        owning_sibling->next_sibling = dest_window;
        dest_window->next_sibling = (window*)0;
        
    } else {
    
        while(owning_sibling->next_sibling != dest_window && owning_sibling)
            owning_sibling = owning_sibling->next_sibling;        
                    
        owning_sibling->next_sibling = dest_window->next_sibling;
        dest_window->next_sibling = (window*)0;
        
        while(owning_sibling->next_sibling)
            owning_sibling = owning_sibling->next_sibling;
            
        owning_sibling->next_sibling = dest_window;
        
        //Go up the tree and make sure the parent is raised
        raiseWindow(dest_window->parent);
    }
    
    //Go up the tree and make sure the parent is raised
    raiseWindow(dest_window->parent);
    
    //Redraw the tree of active windows if we've hit the root 
    if(dest_window->parent->handle == 1)
        drawWindow(dest_window);
}

void raiseHandle(unsigned int handle) {
    
    window* dest_window = getWindowByHandle(handle);
    
    if(!dest_window) {
     
        //cmd_prints("[WYG] Couldn't find the window to be raised\n");   
        return;
    }
    
    raiseWindow(dest_window);
}

void refreshTree() {
    
    drawWindow(&root_window);
}

void destroy(window* dest_window) {

    window* cur_child;
    int i;
            
    //Start by hiding the window 
    markWindowVisible(dest_window, 0);
    
    //Destroy everything that belongs to this window
    cur_child = dest_window->first_child;
    
    while(cur_child) {
        
        destroy(cur_child);
        cur_child = cur_child->next_sibling;
    }
    
    //Now that everything we own has been destroyed, we can destory ourself
    //Find our parent and use that to find the node before us 
    cur_child = dest_window->parent->first_child;
    
    while(cur_child->next_sibling != dest_window && cur_child != dest_window)
        cur_child = cur_child->next_sibling;
        
    if(cur_child == dest_window) {
        
        //The deleted window is the lowest window, so we need to repoint the parent window 
        dest_window->parent->first_child = dest_window->next_sibling;
    } else {
        
        //Otherwise we reparent the next sibling to the previous sibling 
        cur_child->next_sibling = dest_window->next_sibling;
    }
    
    //Remove this bitmap from the handle list     
    for(i = 0; i < window_count; i++) {
        if(registered_windows[i] == dest_window) {
            
            window_count--;
            
            for(; i < window_count; i++)
                registered_windows[i] = registered_windows[i+1];
        
            //This is currently BAD. If we can't realloc, it destroys the entire engine state in the process.    
            if(!(registered_windows = (window**)realloc((void*)registered_windows, sizeof(window*) * (window_count)))) {
                
                //cmd_prints("[WYG] Window list realloc failed\n");
                return;
            }
        
            break;
        }
    }
    
    //Free the context
    freeBitmap(dest_window->context);
    
    //Free the title (if we ever decide to error on unsuccessful frees, this could be an issue for static or undefined titles)
    free((void*)dest_window->title);
    
    //And finally free ourself 
    free((void*)dest_window);
}

void destroyHandle(unsigned int handle) {
    
    window* dest_window = getWindowByHandle(handle);
    
    if(!dest_window) 
        return;
        
    destroy(dest_window);
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
    
    //Start debug console
    //cmd_init(root_window.w, 48);

    //Now we can start the main message loop and begin handling
    //GFX command messages
    while(1) {

        //cmd_prints("[WYG] Waiting for message...");
        getMessage(&temp_msg);
        //cmd_prints("got message ");
        //cmd_printDecimal(temp_msg.command);
        //cmd_pchar('\n');

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
            
            case WYG_GET_LOCATION:
                temp_window = getWindowByHandle(temp_msg.payload);
                postMessage(src_pid, WYG_GET_LOCATION, (unsigned int)((((temp_window->x & 0xFFFF) << 16)) | (temp_window->y & 0xFFFF)));
            break;
            
            case WYG_MOVE_WINDOW:
                current_handle = temp_msg.payload;
                getMessageFrom(&temp_msg, src_pid, WYG_POINT);
                moveWindow(current_handle, (temp_msg.payload & 0xFFFF0000) >> 16, temp_msg.payload & 0xFFFF);
            break;

            case WYG_INSTALL_WINDOW:
                current_handle = temp_msg.payload;
                getMessageFrom(&temp_msg, src_pid, WYG_WHANDLE);
                installWindow(current_handle, temp_msg.payload);
            break;

            case WYG_SHOW_WINDOW:
                markHandleVisible(temp_msg.payload, 1);
            break;
            
            case WYG_RAISE_WINDOW:
                raiseHandle(temp_msg.payload);
            break;

            case WYG_REPAINT_WINDOW:
                drawHandle(temp_msg.payload);
                postMessage(src_pid, WYG_REPAINT_WINDOW, 1);
            break;

            case WYG_SET_TITLE:
                current_handle = temp_msg.payload;
                postMessage(src_pid, WYG_SET_TITLE, 1);
                strlen = getStringLength(src_pid);
                instr = (unsigned char*)malloc(strlen);
                getString(src_pid, instr, strlen);
                setWindowTitle(current_handle, instr);
            break;
            
            case WYG_DESTROY:
                destroyHandle(temp_msg.payload);
                postMessage(src_pid, WYG_DESTROY, 1);
            break;

            default:
            break;
        }
    }
}
