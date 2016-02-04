#ifdef HARNESS_TEST
#include "../../mods/include/p5.h"
#include "../../mods/include/gfx.h"
#include <stdlib.h>
#include <memory.h>
#include <stdio.h>
#include "../../mods/include/wyg.h"
#define REGISTRAR_PID 0
#define REG_DEREGISTER 0
#define SVC_WYG 0
extern char* font_array;
#else 
#include "../include/p5.h"
#include "../include/registrar.h"
#include "../include/gfx.h"
#include "../include/memory.h"
#include "../include/wyg.h"
#include "../include/key.h"
#include "../include/mouse.h"
#include "../vesa/font.h"
#endif //HARNESS_TEST

#define FRAME_SIZE_TOP 28
#define FRAME_SIZE_LEFT 4
#define FRAME_SIZE_BOTTOM 4
#define FRAME_SIZE_RIGHT 4
#define MOUSE_WIDTH 12
#define MOUSE_HEIGHT 12

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
bitmap* old_mouse_bkg;
unsigned short mouse_x;
unsigned short mouse_y;
unsigned char mouse_buffer_ok = 0;

///*!!!!!!!!!! DEBUG SHIT !!!!!!!!!
unsigned char cmd_x;
int cmd_width;
int cmd_max_chars;

void drawCharacter(char c, int x, int y, unsigned int color) {

    setCursor(x, y);
    setColor(color);
    drawChar(c);
}

void cmd_pchar(unsigned char c) {

    if(!inited || cmd_x > cmd_max_chars)
        return;

	drawCharacter(c, (cmd_x*8) + 1, 1, RGB(0, 0, 0));
	cmd_x++;    
}

void cmd_prints(unsigned char* s) {

    if(!inited)
        return;

    setCursor(0, 0);
    setColor(RGB(255, 255, 255));
    fillRect(cmd_width, 14);    

    cmd_x = 0;

    while(*s)
        cmd_pchar(*s++);
}

void cmd_printDecimal(unsigned int dword) {

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

    cmd_pchar(digitToHex((byte & 0xF0)>>4));
    cmd_pchar(digitToHex(byte & 0xF));
}


void cmd_printHexWord(unsigned short wd) {

    cmd_printHexByte((unsigned char)((wd & 0xFF00)>>8));
    cmd_printHexByte((unsigned char)(wd & 0xFF));
}


void cmd_printHexDword(unsigned int dword) {

    cmd_printHexWord((unsigned short)((dword & 0xFFFF0000)>>16));
    cmd_printHexWord((unsigned short)(dword & 0xFFFF));
}

void cmd_init(unsigned short xres, unsigned short yres) {

    cmd_x = 0;
    cmd_width = xres;
    cmd_max_chars = ((cmd_width - 2)/8) - 1;
    inited = 1;
}
//!!!!!!!!!! DEBUG SHIT !!!!!!!!!*/

void drawWindow(window* cur_window, unsigned char use_current_blit);
void raiseWindow(window* dest_window);
void drawFrame(window* cur_window);

void bmpDrawHLine(bitmap* bmp, int x, int y, int length, unsigned int color) {

    int i, endx;

    endx = x + length;

    for(i = x; i < endx; i++)
        bmp->data[y*bmp->width + i] = color;
}

void bmpDrawVLine(bitmap* bmp, int x, int y, int length, unsigned int color) {

    int i, endy;

    endy = length + y;

    for(i = y; i < endy; i++)
        bmp->data[i*bmp->width + x] = color;
}


void bmpDrawRect(bitmap* bmp, int x, int y, int width, int height, unsigned int color) {

    bmpDrawHLine(bmp, x, y, width, color);
    bmpDrawVLine(bmp, x, y, height, color);
    bmpDrawHLine(bmp, x, y + height - 1, width, color);
    bmpDrawVLine(bmp, x + width - 1, y, height, color);
}


void bmpFillRect(bitmap* bmp, int x, int y, int width, int height, unsigned int color) {

    int j, i;
    int endx, endy;

    endx = width + x;
    endy = height + y;

    //for(i = 0; i < (bmp->height*bmp->width); i++)
    //    bmp->data[i] = RGB(255, 0, 0);

    //return;

    for(i = y; i < endy; i++) {

        for(j = x; j < endx; j++) {

            bmp->data[i*bmp->width + j] = color; 
        }
    }
}


void bmpDrawCharacter(bitmap* bmp, unsigned char c, int x, int y, unsigned int color) {

    return;

    int j, i;
    unsigned char line;
    c = c & 0x7F; //Reduce to base ASCII set

    for(i = 0; i < 12; i++) {

        //prints("Reading a line from font cache...");
        line = font_array[i * 128 + c];
        //prints("done\n");
        for(j = 0; j < 8; j++) {
            
            if(line & 0x80) bmp->data[(y + i)*bmp->width + (x + j)] = color; 
            line = line << 1;
        }
    }
}

void displayString(int x, int y, unsigned char* s) {
        
    while((*s)) {
        
        setCursor(x, y);
        drawChar(*(s++));
        
        x += 8;
    }
}

void eraseMouse() {
 
    //prints("Erasing mouse\n");
    return;
    
    if(!mouse_buffer_ok)
        return;
        
    setCursor(mouse_x, mouse_y);
    drawBitmap(old_mouse_bkg);
}

void drawMouse() {
    
    //prints("Drawing mouse\n");
    return;
    
    setCursor(mouse_x, mouse_y);
    copyScreen(old_mouse_bkg);
    
    setColor(0);
    setCursor(mouse_x + (MOUSE_WIDTH / 2), mouse_y);
    drawVLine(MOUSE_HEIGHT);
    setCursor(mouse_x + (MOUSE_WIDTH / 2) - 1, mouse_y);
    drawVLine(MOUSE_HEIGHT);
    setCursor(mouse_x, mouse_y + (MOUSE_HEIGHT / 2));
    drawHLine(MOUSE_WIDTH);
    setCursor(mouse_x, mouse_y + (MOUSE_HEIGHT / 2) - 1);
    drawHLine(MOUSE_WIDTH);
    
    mouse_buffer_ok = 1;
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

//#define RECT_TEST 1
void drawBmpRect(window* win, rect r) {

#ifdef RECT_TEST 
  
    setColor(RGB(0, 255, 0));
    setCursor(r.left, r.top);
    drawRect(r.right - r.left, r.bottom - r.top);
    
#else     

    //Adjust the rectangle coordinate from global space to window space 
    win->context->top = r.top - win->y;
    win->context->left = r.left - win->x;
    win->context->bottom = r.bottom - win->y;
    win->context->right = r.right - win->x;   
    
    //Do the blit
    setCursor(win->x, win->y);
    drawBitmap(win->context);
    
#endif //RECT_TEST
}

rect* splitRect(rect rdest, rect rknife, int* out_count) {
	
	rect baserect;
	rect* outrect;
	int rect_count = 0;
	unsigned int i;
	
	baserect.top = rdest.top;
	baserect.left = rdest.left;
	baserect.bottom = rdest.bottom;
	baserect.right = rdest.right;

#ifdef RECT_TEST    
    //printf("splitting (%u, %u, %u, %u)", baserect.top, baserect.left, baserect.bottom, baserect.right);
    //printf("against (%u, %u, %u, %u)\n", rknife.top, rknife.left, rknife.bottom, rknife.right);
#endif //RECT_TEST

	if(rknife.left > baserect.left && rknife.left < baserect.right)
		rect_count++;
		
	if(rknife.right > baserect.left && rknife.right < baserect.right)
		rect_count++;
	
	if(rknife.top < baserect.bottom && rknife.top > baserect.top)
		rect_count++;		
		
	if(rknife.bottom < baserect.bottom && rknife.bottom > baserect.top)
		rect_count++;	
	
	if(rect_count == 0) {
		
		out_count[0] = 0;
		return (rect*)0;
	}
		
    //prints("Allocating space for ");
     //printDecimal(sizeof(rect)*rect_count);
    //prints(" rect bytes\n");
	outrect = (rect*)malloc(sizeof(rect)*rect_count);
    if(!outrect)
        prints("Couldn't allocate rect space\n");
	rect_count = 0;
	
    //prints("Doing left edge split\n");
	//Split by left edge
	if(rknife.left > baserect.left && rknife.left < baserect.right) {
		
		outrect[rect_count].top = baserect.top;
		outrect[rect_count].bottom = baserect.bottom;
		outrect[rect_count].right = rknife.left - 1;
		outrect[rect_count].left = baserect.left;
		
		baserect.left = rknife.left;
		
		rect_count++;
	}

    //prints("Doing top edge split\n");
	//Split by top edge
	if(rknife.top < baserect.bottom && rknife.top > baserect.top) {
		
		outrect[rect_count].top = baserect.top;
		outrect[rect_count].bottom = rknife.top - 1;
		outrect[rect_count].right = baserect.right;
		outrect[rect_count].left = baserect.left;
		
		baserect.top = rknife.top;
		
		rect_count++;
	}

    //prints("Doing right edge split\n");
	//Split by right edge
	if(rknife.right > baserect.left && rknife.right < baserect.right) {
		
		outrect[rect_count].top = baserect.top;
		outrect[rect_count].bottom = baserect.bottom;
		outrect[rect_count].right = baserect.right;
		outrect[rect_count].left = rknife.right + 1;
		
		baserect.right = rknife.right;
		
		rect_count++;
	}

    //prints("Doing bottom edge split\n");
	//Split by right edge
	if(rknife.bottom > baserect.top && rknife.bottom < baserect.bottom) {
		
		outrect[rect_count].top = rknife.bottom + 1;
		outrect[rect_count].bottom = baserect.bottom;
		outrect[rect_count].right = baserect.right;
		outrect[rect_count].left = baserect.left;
		
		baserect.bottom = rknife.bottom;
		
		rect_count++;
	}

	out_count[0] = rect_count;

	return outrect;	
}

void drawOccluded(window* win, rect baserect, rect* splitrects, int rect_count) {
	
	int split_count = 0;
	int total_count = 1;
	int working_total = 0;
	rect* out_rects = (rect*)0;
	rect* working_rects = (rect*)0;
	int i, j, k;

#ifdef RECT_TEST
	
    //Clear everything 
    setColor(RGB(255, 255, 255));
    setCursor(0, 0);
    fillRect(root_window.context->width, root_window.context->height);
    
    //Draw the base window
    setColor(RGB(0, 0, 255));
    setCursor(baserect.left, baserect.top);
    drawRect(baserect.right - baserect.left, baserect.bottom - baserect.top);
    
    //Draw the overlapping windows
    for(i = 0; i < rect_count; i++) {
        
        setColor(RGB(255, 0, 0));
        setCursor(splitrects[i].left, splitrects[i].top);
        drawRect(splitrects[i].right - splitrects[i].left, splitrects[i].bottom - splitrects[i].top);
    }
    
#endif //RECT_TEST
    
    //If there's nothing occluding us, just render the bitmap and get out of here
    if(!rect_count) {
    
        //prints("[WYG] Nothing overlapping us\n");
        drawBmpRect(win, baserect);
        return;
    }
    
    //prints("[WYG] Allocating space for output rectangles\n");
	out_rects = (rect*)malloc(sizeof(rect));
    if(!out_rects)
        prints("[WYG] Couldn't allocate memory\n");
	out_rects[0].top = baserect.top;
	out_rects[0].left = baserect.left;
	out_rects[0].bottom = baserect.bottom;
	out_rects[0].right = baserect.right;
	
	//For each splitting rect, split each rect in out_rects, delete the rectangle that was split, and add the resultant split rectangles
	for(i = 0; i < rect_count; i++) {
        
		for(j = 0; j < total_count;) {
            
			//Only bother with this combination of rectangles if the rectangle to be split (out_rects[]) 
			//is not a blank (zero-dimension) -- we don't have to check for intersection as the calling function
            //already does that
			if(!(out_rects[j].top == 0 &&
				 out_rects[j].left == 0 &&
				 out_rects[j].bottom == 0 &&
				 out_rects[j].right == 0)&&
                 (splitrects[i].left <= out_rects[j].right &&
			      splitrects[i].right >= out_rects[j].left &&
			      splitrects[i].top <= out_rects[j].bottom && 
			      splitrects[i].bottom >= out_rects[j].top)) {
			
				rect* split_rects = splitRect(out_rects[j], splitrects[i], &split_count);

#ifdef RECT_TEST
			            
                for(k = 0; k < split_count; k++)
                    //printf("split %u, %u, %u, %u\n", split_rects[k].top, split_rects[k].left, split_rects[k].bottom, split_rects[k].right);

#endif //RECT_TEST
            
				//If nothing was returned, we actually want to clip a rectangle in its entirety
				if(!split_count) {
					
					//We use zero-dimension rects to represent blank entries
					//Ideally we would want to break out of the function the second
					//we've rejected the last rect, but in this case our memory model
					//makes that a little difficult. We could do this very quickly 
					//if we switched to some kind of linked list
					out_rects[j].top = 0;
					out_rects[j].left = 0;
					out_rects[j].bottom = 0;
					out_rects[j].right = 0;
					
					j++;
					continue;
				}
				
				//From here on out, the first result rectangle is alredy allocated for because it takes the
				//place of the rectangle that was split
				split_count--;
                //prints("[WYG] Reallocating space for output rectangles\n");
				out_rects = (rect*)realloc(out_rects, sizeof(rect) * (split_count + total_count));
                if(!out_rects)
                    prints("[WYG] Couldn't allocate memory\n");
					
				//Replace the rectangle that got split with the first result rectangle 
				out_rects[j].top = split_rects[0].top;
				out_rects[j].left = split_rects[0].left;
				out_rects[j].bottom = split_rects[0].bottom;
				out_rects[j].right = split_rects[0].right;
				
				//Append the rest of the result rectangles to the output collection
				for(k = 0; k < split_count; k++) {
					
					out_rects[total_count + k].top = split_rects[k+1].top;
					out_rects[total_count + k].left = split_rects[k+1].left;
					out_rects[total_count + k].bottom = split_rects[k+1].bottom;
					out_rects[total_count + k].right = split_rects[k+1].right;
				}
				
				//Free the space that was used for the split 
				free(split_rects);
				
				//Update the count of rectangles 
				total_count += split_count;
				
				//Restart the list 
				j = 0;
			} else {
				
                //prints("Not a real rect\n");
				j++;
			}
		}
	}
	
    for(k = 0; k < total_count; k++) {

#ifdef RECT_TEST    
        //printf("%u, %u, %u, %u\n", out_rects[k].top, out_rects[k].left, out_rects[k].bottom, out_rects[k].right);
#endif //RECT_TEST
    
        if(!(out_rects[k].top == 0 &&
             out_rects[k].left == 0 &&
             out_rects[k].bottom == 0 &&
             out_rects[k].right == 0)) {
                    
                drawBmpRect(win, out_rects[k]);
             }
    }
		
	free(out_rects);
}

unsigned int newWindow(unsigned int width, unsigned int height, unsigned char flags, unsigned int pid) {
    
    window* new_window;
    unsigned int i, bufsz;
    
	cmd_prints("Creating a new window"); 
	
    if(!(new_window = (window*)malloc(sizeof(window)))) {
        
         prints("[WYG] Couldn't allocate a new window\n");
        return 0;
    }
    
    //This is currently BAD. If we can't realloc, it destroys the entire engine state in the process.    
    if(!(registered_windows = (window**)realloc((void*)registered_windows, sizeof(window*) * (window_count + 1)))) {
        
         prints("[WYG] Window list realloc failed\n");
        return 0;
    }
    
     //prints("[WYG] Created new window, setting initial values\n");
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
        
         //prints("[WYG] Could not create a new window context\n");
        free((void*)new_window);
        return 0;
    } 
    
    bufsz = new_window->w * new_window->h;
    
    //Clear new window to white
    for(i = 0; i < bufsz; i++)
        new_window->context->data[i] = RGB(255, 255 ,255);
        
     //prints("[WYG] Installing new window into window list\n");
    new_window->handle = next_handle++;
    registered_windows[window_count++] = new_window;
    
     //prints("[WYG] Successfully created new window ");
      //printDecimal(new_window->handle);
     //pchar('\n');
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

bitmap* getWindowContext(unsigned int handle) {
    
    window* dest_window = getWindowByHandle(handle);
    
    if(!dest_window) {
     
         //prints("[WYG] Couldn't find the window to get its context\n");   
        return (bitmap*)0;
    }
        
    return dest_window->context;
}

//Redraws every window intersected by window_bounds
void updateOverlapped(rect* window_bounds) {
    
    int i;
    rect comp_rect, draw_rect; 
    
    //prints("[WYG] Looking for previously overlapped windows\n");
        
    for(i = 0; i < window_count; i++) {
        
        comp_rect.top = registered_windows[i]->y;
        comp_rect.left = registered_windows[i]->x;
        comp_rect.bottom = comp_rect.top + registered_windows[i]->h - 1;
        comp_rect.right = comp_rect.left + registered_windows[i]->w - 1;
        
        if(window_bounds->left <= comp_rect.right &&
           window_bounds->right >= comp_rect.left &&
           window_bounds->top <= comp_rect.bottom && 
           window_bounds->bottom >= comp_rect.top && 
           (registered_windows[i]->flags & WIN_VISIBLE)) {
            
            //prints("[WYG] Found an overlapped window\n");   
            if(window_bounds->top < comp_rect.top)
                draw_rect.top = comp_rect.top; 
            else 
                draw_rect.top = window_bounds->top;
                
            if(window_bounds->left < comp_rect.left)
                draw_rect.left = comp_rect.left; 
            else 
                draw_rect.left = window_bounds->left;
                
            if(window_bounds->bottom > comp_rect.bottom)
                draw_rect.bottom = comp_rect.bottom; 
            else 
                draw_rect.bottom = window_bounds->bottom;
                
            if(window_bounds->right > comp_rect.right)
                draw_rect.right = comp_rect.right; 
            else 
                draw_rect.right = window_bounds->right;
            
            registered_windows[i]->context->top = draw_rect.top - registered_windows[i]->y;
            registered_windows[i]->context->left = draw_rect.left - registered_windows[i]->x;       
            registered_windows[i]->context->bottom = draw_rect.bottom - registered_windows[i]->y;
            registered_windows[i]->context->right = draw_rect.right - registered_windows[i]->x;        
            drawWindow(registered_windows[i], 1);
        }
    }
}

void moveWindow(window* dest_window, unsigned short new_x, unsigned short new_y) {
    
    rect overlap_rect;
            
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
        drawWindow(dest_window, 0);
    } 
        
    return;
}

void moveHandle(unsigned int handle, unsigned short new_x, unsigned short new_y) {
    
    window* dest_window = getWindowByHandle(handle);
    
    if(!dest_window) {
     
         //prints("[WYG] Couldn't find window to mark it visible\n");   
        return;
    }
    
    eraseMouse();
    
    moveWindow(dest_window, new_x, new_y);
    
    drawMouse();
}

void installWindow(unsigned int child_handle, unsigned int parent_handle) {
    
    window* child_window = getWindowByHandle(child_handle);
    window* parent_window = getWindowByHandle(parent_handle);
    window* sibling_window;
    
    if(!child_window || !parent_window) {
     
         //prints("[WYG] Couldn't find the parent or child window to perform window install\n");   
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

    if(is_visible) {
                
        if(!(dest_window->flags & WIN_UNDECORATED)) {
         
            drawFrame(dest_window);
        }
               
        dest_window->flags |= WIN_VISIBLE;
    } else {
        
        dest_window->flags &= ~((unsigned char)WIN_VISIBLE);
    }
    
    if(was_visible && !is_visible) {
        
        overlap_rect.top = dest_window->y;
        overlap_rect.left = dest_window->x;
        overlap_rect.bottom = overlap_rect.top + dest_window->h - 1;
        overlap_rect.right = overlap_rect.left + dest_window->w - 1;
        
        updateOverlapped(&overlap_rect); //Redraw all of the siblings that this window was covering up
    } else {
        
        drawWindow(dest_window, 0);
    }
    
    return;
}

void markHandleVisible(unsigned int handle, unsigned char is_visible) {
    
    window* dest_window = getWindowByHandle(handle);
    
    if(!dest_window) {
     
         //prints("[WYG] Couldn't find window to mark it visible\n");   
        return;
    }
    
    eraseMouse();
    
    markWindowVisible(dest_window, is_visible);
    
    drawMouse();
}

void markWindowDirty(unsigned int handle) {
    
    window* dest_window = getWindowByHandle(handle);
    
    if(!dest_window) {
     
         //prints("[WYG] Couldn't find window to mark it dirty\n");   
        return;
    }
        
    dest_window->needs_redraw = 1;
    
    return;
}

void setWindowTitle(unsigned int handle, unsigned char* newstr) {
    
    window* dest_window = getWindowByHandle(handle);
    
    if(!dest_window) {
     
         //prints("[WYG] Couldn't find window to mark it dirty\n");   
        return;
    }
    
    if(dest_window->title)
        free(dest_window->title);
        
    dest_window->title = newstr;
    
    dest_window->frame_needs_redraw = 1;
}

void bmpDrawPanel(bitmap* bmp, int x, int y, int width, int height, unsigned int color, int border_width, int invert) {

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
        bmpDrawHLine(bmp, x+i, y+i, width-(2*i), light_color);

        //Left edge
        bmpDrawVLine(bmp, x+i, y+i+1, height-((i+1)*2), light_color);

        //Bottom edge
        bmpDrawHLine(bmp, x+i, (y+height)-(i+1), width-(2*i), shade_color);

        //Right edge
        bmpDrawVLine(bmp, x+width-i-1, y+i+1, height-((i+1)*2), shade_color);
    }
}

void drawTitlebar(window* cur_window, unsigned char active) {
    
    unsigned char* s;    
    unsigned int tb_color, text_color;
    rect old_ctx_rect;
        
    //Titlebar
    if(active)
        tb_color = RGB(182, 0, 0);
    else 
        tb_color = RGB(238, 203, 137);
    
    bmpFillRect(cur_window->context, 4, 4, cur_window->w - 28, 20, tb_color);
        
     //Window title
    if(cur_window->title) {
        
         //prints(cur_window->title);
        
        int base_x, base_y, off_x, titlebar_width;
        
        s = cur_window->title;
        base_x = 7;
        base_y = 9;
        off_x = 0;
        titlebar_width = cur_window->w - 28;
        
        if(active)
            text_color = RGB(255, 255, 255);
        else
            text_color = RGB(138, 103, 37);
        
        while(*s) {
            bmpDrawCharacter(cur_window->context, *(s++), base_x + off_x, base_y, text_color);
            off_x += 8;
            
            //Truncate the text if it's wider than the titlebar
            if(off_x >= titlebar_width)
                break;
        }
    }
    
    /*
    old_ctx_rect.top = cur_window->context->top;
    old_ctx_rect.left = cur_window->context->left;
    old_ctx_rect.bottom = cur_window->context->bottom;
    old_ctx_rect.right = cur_window->context->right;
    
    cur_window->context->top = 4;
    cur_window->context->left = 4;
    cur_window->context->bottom = 23;
    cur_window->context->right = cur_window->context->right - 25;
    
    drawWindow(cur_window, 1);
    
    cur_window->context->top = old_ctx_rect.top;
    cur_window->context->left = old_ctx_rect.left;
    cur_window->context->bottom = old_ctx_rect.bottom;
    cur_window->context->right = old_ctx_rect.right;
    */
}

void drawFrame(window* cur_window) {
    
    int i;
    
    
     //prints("[WYG] Drawing frame for window ");
      //printDecimal(cur_window->handle);
     //pchar('\n');
    
    //Outer border
    bmpDrawPanel(cur_window->context, 0, 0, cur_window->w, cur_window->h, RGB(238, 203, 137), 1, 0);
    
    //Title border
    bmpDrawPanel(cur_window->context, 3, 3, cur_window->w - 6, 22, RGB(238, 203, 137), 1, 1);
    
    //Body border
    bmpDrawPanel(cur_window->context, 3, 27, cur_window->w - 6, cur_window->h - 32, RGB(238, 203, 137), 1, 1);
    
    //Left frame
    bmpFillRect(cur_window->context, 1, 1, 2, cur_window->h - 2, RGB(238, 203, 137)); 
    
    //Right frame
    bmpFillRect(cur_window->context, cur_window->w - 3, 1, 2, cur_window->h - 2, RGB(238, 203, 137)); 
    
    //Top frame
    bmpFillRect(cur_window->context, 3, 1, cur_window->w - 6, 2, RGB(238, 203, 137)); 
    
    //Mid frame
    bmpFillRect(cur_window->context, 3, 25, cur_window->w - 6, 2, RGB(238, 203, 137)); 
    
    //Bottom frame
    bmpFillRect(cur_window->context, 3, cur_window->h - 3, cur_window->w - 6, 2, RGB(238, 203, 137)); 
        
    //Button
    bmpDrawPanel(cur_window->context, cur_window->w - 24, 4, 20, 20, RGB(238, 203, 137), 1, 0);
    bmpFillRect(cur_window->context, cur_window->w - 23, 5, 18, 18, RGB(238, 203, 137)); 
    
    drawTitlebar(cur_window, cur_window->next_sibling == (window*)0);
    
    cur_window->frame_needs_redraw = 0;
}

//   Developing a proper redraw routine is going to be one of
//the most key aspects of getting a responsive window manager.
//One of the current major issues we're experiencing thus far
//is that small, repeated redraw requests are filling up the 
//message buffer and slowing everything to a crawl (this is 
//currently happening on our terminal which is redrawing with
//each //printed character)
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

rect* getOverlappingWindows(window* cur_window, unsigned int* rect_count, rect* rect_collection, rect* baserect, unsigned char initial, unsigned char create_rects) {

    rect* return_rects = (rect*)0;

    //See if I overlap, and then check my children 
    if(cur_window) {
        
        //Allocate space for rectangles if we haven't yet AND we're building them
        if(create_rects && !rect_collection) {
            
            return_rects = (rect*)malloc(sizeof(rect)*(rect_count[0]));
            if(!return_rects)
                prints("[WYG] Couldn't allocate space for the rectangles\n");
            rect_count[0] = 0;
        } else {
            
            return_rects = rect_collection;
        }
        
        //Count the window only if it overlaps
        if(!initial && /* Don't check the current window if it's the overlapped window */
           cur_window->x <= baserect->right &&
           (cur_window->x + cur_window->context->width - 1) >= baserect->left &&
           cur_window->y <= baserect->bottom && 
           (cur_window->y + cur_window->context->height - 1) >= baserect->top) {
           
                    //Create the rectangle, if we're into that junk
                if(create_rects) {
                    
                    return_rects[rect_count[0]].top = cur_window->y;
                    return_rects[rect_count[0]].left = cur_window->x;
                    return_rects[rect_count[0]].bottom = (cur_window->y + cur_window->context->height - 1);
                    return_rects[rect_count[0]].right = (cur_window->x + cur_window->context->width - 1);
                }
               
                rect_count[0]++;
           }
        
        //Get the overlapping children
        getOverlappingWindows(cur_window->first_child, rect_count, return_rects, baserect, 0, create_rects);
        
        //Get the overlapping higher sibling windows 
        getOverlappingWindows(cur_window->next_sibling, rect_count, return_rects, baserect, 0, create_rects);
    }
    
    return return_rects;
}

void drawWindow(window* cur_window, unsigned char use_current_blit) {
     
    unsigned int rect_count;
    rect* splitrects;
    rect winrect;
    int i;
    
     //prints("[WYG] Drawing window ");
      //printDecimal(cur_window->handle);
     //pchar('\n');
    
    if(cur_window->flags & WIN_VISIBLE) {
        
        cur_window->needs_redraw = 0;
        
        //Start by drawing this window
        //prints("[WYG] Drawing window frame\n");
        
        //Create a rectangle for the window to be drawn
        if(use_current_blit) {
            
            //prints("[WYG] Setting base rectangle using winrect\n");
            //Convert the current blit window to desktop space
            winrect.top = cur_window->y + cur_window->context->top;
            winrect.left = cur_window->x + cur_window->context->left;
            winrect.bottom = cur_window->y + cur_window->context->bottom;
            winrect.right = cur_window->x + cur_window->context->right;
        } else {
                
            //prints("[WYG] Setting base rectangle using whole ctx\n");
            winrect.top = cur_window->y;
            winrect.left = cur_window->x;
            winrect.bottom = cur_window->y + cur_window->context->height - 1;
            winrect.right = cur_window->x + cur_window->context->width - 1;
        }
        
        rect_count = 0;
        //prints("[WYG] Counting overlapping windows\n");
        getOverlappingWindows(cur_window, &rect_count, (rect*)0, &winrect, 1, 0); //count the rects 
        //prints("[WYG] Building overlapping rectangles\n");
        splitrects = getOverlappingWindows(cur_window, &rect_count, (rect*)0, &winrect, 1, 1); //build the rects        
        //prints("[WYG] Drawing occluded window\n");
        drawOccluded(cur_window, winrect, splitrects, rect_count);   
        //prints("[WYG] Finished doing occluded draw\n");    
        
        //getch();
                
        free(splitrects);       
    }
    
     //prints("[WYG] Finished drawing window ");
      //printDecimal(cur_window->handle);
     //pchar('\n');
    
    return;
}

void drawHandle(unsigned int handle) {
    
    window* dest_window = getWindowByHandle(handle);
    
    if(!dest_window) {
     
         //prints("[WYG] Couldn't find the window to be raised\n");   
        return;
    }
    
    eraseMouse();
    
    //Draw the window, assume we want to use the blit window set up by the client   
    drawWindow(dest_window, 1);
    
    drawMouse();
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
    if(dest_window->parent->handle == 1) {
    
        dest_window->frame_needs_redraw;    
        drawWindow(dest_window, 0);
    }
}

void raiseHandle(unsigned int handle) {
    
    window* dest_window = getWindowByHandle(handle);
    
    if(!dest_window) {
     
         //prints("[WYG] Couldn't find the window to be raised\n");   
        return;
    }
    
    eraseMouse();
    
    raiseWindow(dest_window);
    
    drawMouse();
}

void refreshTree() {
    
    eraseMouse();
    
    drawWindow(&root_window, 0);
    
    drawMouse();
}

void destroy(window* dest_window) {

    window *cur_child, *next;
    int i;
            
    //Start by hiding the window 
    markWindowVisible(dest_window, 0);
    
    //Destroy everything that belongs to this window
    cur_child = dest_window->first_child;
    
    while(cur_child) {
        
        next = cur_child->next_sibling;
        destroy(cur_child);
        cur_child = next;
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
                
                 prints("[WYG] Window list realloc failed\n");
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
    
    eraseMouse();
        
    destroy(dest_window);
    
    drawMouse();
}

#ifdef HARNESS_TEST
void WYG_main(void) {
#else 
void main(void) {
#endif //HARNESS_TEST

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

#ifndef HARNESS_TEST

    //Get the 'here's my pid' message from init
    getMessage(&temp_msg);
    parent_pid = temp_msg.source;
    //prints("[WYG] Starting WYG GUI services.\n");

    //First thing, register as a WYG service with the registrar
    postMessage(REGISTRAR_PID, REG_REGISTER, SVC_WYG);
    getMessage(&temp_msg);

    if(!temp_msg.payload) {

        //prints("\n[WYG] failed to register WYG service.\n");
        postMessage(REGISTRAR_PID, REG_DEREGISTER, SVC_WYG);
        postMessage(parent_pid, 0, 0); //Tell the parent we're done registering
        terminate();
    }

    if(!initKey()) {
        
        postMessage(REGISTRAR_PID, REG_DEREGISTER, SVC_WYG);
        postMessage(parent_pid, 0, 0); //Tell the parent we're done registering
        terminate();
    }

    if(!initMouse()) {
        
        //Don't need to terminate, but do need to display a warning to the user
    }

#endif //HARNESS_TEST

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

    if(num) {

        mode = getModeDetails(num);
    } else {

        //prints("[WYG] Staying in text mode.\n");
        postMessage(REGISTRAR_PID, REG_DEREGISTER, SVC_WYG);
        postMessage(parent_pid, 0, 0); //Tell the parent we're done registering
        terminate();
    }
    
	cmd_init(mode->width, mode->height);
	
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
    root_window.y = 14;
    root_window.w = mode->width;
    root_window.h = mode->height - 14;
    
    //Create a drawing context for the root window
    if(!(root_window.context = newBitmap(root_window.w, root_window.h))) {
        
        //prints("[WYG] Could not allocate a context for the root window.\n");
        free((void*)registered_windows);
        postMessage(REGISTRAR_PID, REG_DEREGISTER, SVC_WYG);
        postMessage(parent_pid, 0, 0); //Tell the parent we're done registering
        terminate();
    } 

    //Set up the initial mouse position
    mouse_x = root_window.w / 2 - 1;
    mouse_y = root_window.h / 2 - 1;

    //Create a bitmap to store the mouse dirtyrect
    if(!(old_mouse_bkg = newBitmap(MOUSE_WIDTH, MOUSE_HEIGHT))) {
        
        //prints("[WYG] Could not allocate a context for the mouse sirty buffer.\n");
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
    //init(root_window.w, 48);

#ifdef HARNESS_TEST

    //enter the testing code
    testMain();
    endGfx();
    return;

#else 

    //Now we can start the main message loop 
    while(1) {

        //prints("[WYG] Waiting for message...");
        getMessage(&temp_msg);
        //prints("got message ");
         //printDecimal(temp_msg.command);
        ////pchar('\n');

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
                moveHandle(current_handle, (temp_msg.payload & 0xFFFF0000) >> 16, temp_msg.payload & 0xFFFF);
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
            
            case WYG_GET_FRAME_DIMS:
                postMessage(src_pid, WYG_GET_FRAME_DIMS, (FRAME_SIZE_TOP << 24) | (FRAME_SIZE_LEFT << 16) | (FRAME_SIZE_BOTTOM << 8) | (FRAME_SIZE_RIGHT));
            break;

            case MOUSE_SEND_UPDATE:
                setColor(RGB(255, 0, 0));
                displayString(0, 0, "GOT A MOUSE EVENT!");
                while(1);
            break;

            default:
            break;
        }
    }
    
#endif //HARNESS_TEST
    
}
