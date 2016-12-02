#ifdef HARNESS_TEST
#include "../p5-redux/P5OSPPB/mods/include/gfx.h"
#include "../p5-redux/P5OSPPB/mods/include/p5.h"
#include "../p5-redux/P5OSPPB/mods/include/key.h"
#include <stdlib.h>
#include <memory.h>
#include <stdio.h>
#include <emscripten.h>
#include "../p5-redux/P5OSPPB/mods/include/wyg.h"
#define REGISTRAR_PID 0
#define REG_DEREGISTER 0
#define SVC_WYG 0
extern char* font_array;
extern void testMain();
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
#include "list.h"
#include "rect.h"

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

void window_printer(void* value);

message temp_msg;

unsigned char inbuf[12];

typedef struct window {
	unsigned char active;
    unsigned char flags;
    unsigned int handle;
    unsigned int pid;
    bitmap* context;
    unsigned int w;
    unsigned int h;
    unsigned int x;
    unsigned int y;
    unsigned char needs_redraw;
    unsigned char* title;
    unsigned char frame_needs_redraw;
} window;

window *root_window, *mouse_window;
List* window_list;
unsigned char inited = 0;
bitmap* old_mouse_bkg;
short mouse_x;
short mouse_y;
unsigned char mouse_buffer_ok = 0;
unsigned int alloc_ta, alloc_time;
unsigned int draw_ta, draw_time;
bitmap* back_buffer;

/*!!!!!!!!!! DEBUG SHIT !!!!!!!!!
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

void cons_init();
void cons_putc(char c);
void cons_prints(char* s);
void cons_printDecimal(unsigned int dword);
void cons_printHexByte(unsigned char byte);
void cons_printHexWord(unsigned short wd);
void cons_printHexDword(unsigned int dword);

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
!!!!!!!!!! DEBUG SHIT !!!!!!!!!*/

void drawWindow(window* cur_window, unsigned char use_current_blit);
void raiseWindow(window* dest_window);
void drawFrame(window* cur_window);
void drawTitlebar(window* cur_window, int do_refresh);

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
void drawBmpRect(window* win, Rect* r) {

    int x, y;
    unsigned int tmp;

#ifdef RECT_TEST 
  
    setColor(RGB(0, 255, 0));
    setCursor(r->left, r->top);
    drawRect(r->right - r->left, r->bottom - r->top);
    
#else     

    //draw_ta = getElapsedMs();
 
    //Adjust the rectangle coordinate from global space to window space 
    win->context->top = r->top - win->y;
    win->context->left = r->left - win->x;
    win->context->bottom = r->bottom - win->y;
    win->context->right = r->right - win->x;   
    
    //Do the blit
    //setCursor(win->x, win->y);
    //drawBitmap(win->context);

    for(y = r->top; y <= r->bottom && y < root_window->h; y++) {

        memcpy(&win->context->data[((y - win->y) * win->w) + (r->left - win->x)], 
               &back_buffer->data[(y * back_buffer->width) + (r->left)],
               (win->w > back_buffer->width ? back_buffer->width : win->w) * 4);
    }

    //draw_time += getElapsedMs() - draw_ta; 

#endif //RECT_TEST
}

List* splitRect(Rect* rdest, Rect* rknife) {
	
	Rect baserect;
	List* outrect;
	Rect* new_rect;
	
	baserect.top = rdest->top;
	baserect.left = rdest->left;
	baserect.bottom = rdest->bottom;
	baserect.right = rdest->right;
	
/*
	cons_prints("Splitting rect (");
	cons_printDecimal(rdest->top);
	cons_prints(", ");
	cons_printDecimal(rdest->left);
	cons_prints(", ");
	cons_printDecimal(rdest->bottom);
	cons_prints(", ");
	cons_printDecimal(rdest->right);
	cons_prints(") with (");   
	cons_printDecimal(rknife->top);
	cons_prints(", ");
	cons_printDecimal(rknife->left);
	cons_prints(", ");
	cons_printDecimal(rknife->bottom);
	cons_prints(", ");
	cons_printDecimal(rknife->right);
	cons_prints(")\n");
*/
	
#ifdef RECT_TEST    
    //printf("splitting (%u, %u, %u, %u)", baserect.top, baserect.left, baserect.bottom, baserect.right);
    //printf("against (%u, %u, %u, %u)\n", rknife.top, rknife.left, rknife.bottom, rknife.right);
#endif //RECT_TEST
		
    //prints("Allocating space for ");
     //printDecimal(sizeof(rect)*rect_count);
    //prints(" rect bytes\n");
	outrect = List_new();
    if(!outrect) {
		
        prints("Couldn't allocate rect space\n");
		return outrect;
	}
	
//    cons_prints("Doing left edge split\n");
	//Split by left edge
	if(rknife->left >= baserect.left && rknife->left <= baserect.right) {
		
		new_rect = Rect_new(baserect.top, baserect.left, baserect.bottom, rknife->left - 1);
		
		if(!new_rect) {
			
			List_delete(outrect, Rect_deleter);
			return (List*)0;
		}
		
		if(!List_add(outrect, new_rect)) {
			
			free((void*)new_rect);
			List_delete(outrect, Rect_deleter);
			return (List*)0;
		}
		
		baserect.left = rknife->left;
	}

//    cons_prints("Doing top edge split\n");
	//Split by top edge
	if(rknife->top <= baserect.bottom && rknife->top >= baserect.top) {
		
		new_rect = Rect_new(baserect.top, baserect.left, rknife->top - 1, baserect.right);
		
		if(!new_rect) {
			
			List_delete(outrect, Rect_deleter);
			return (List*)0;
		}
		
		if(!List_add(outrect, new_rect)) {
			
			free((void*)new_rect);
			List_delete(outrect, Rect_deleter);
			return (List*)0;
		}
		
		baserect.top = rknife->top;
	}

//    cons_prints("Doing right edge split\n");
	//Split by right edge
	if(rknife->right >= baserect.left && rknife->right <= baserect.right) {
		
		new_rect = Rect_new(baserect.top, rknife->right + 1, baserect.bottom, baserect.right);
		
		if(!new_rect) {
			
			List_delete(outrect, Rect_deleter);
			return (List*)0;
		}
		
		if(!List_add(outrect, new_rect)) {
			
			free((void*)new_rect);
			List_delete(outrect, Rect_deleter);
			return (List*)0;
		}
		
		baserect.right = rknife->right;
	}

//    cons_prints("Doing bottom edge split\n");
	//Split by bottom edge
	if(rknife->bottom >= baserect.top && rknife->bottom <= baserect.bottom) {
		
		new_rect = Rect_new(rknife->bottom + 1, baserect.left, baserect.bottom, baserect.right);
		
		if(!new_rect) {
			
			List_delete(outrect, Rect_deleter);
			return (List*)0;
		}
		
		if(!List_add(outrect, new_rect)) {
			
			free((void*)new_rect);
			List_delete(outrect, Rect_deleter);
			return (List*)0;
		}
		
		baserect.bottom = rknife->bottom;
	}

/*    cons_prints("Result: \n");
	
	List_for_each(outrect, new_rect, Rect*) {
	
	    cons_prints("    ");
		cons_printDecimal(new_rect->top);
		cons_prints(", ");
		cons_printDecimal(new_rect->left);
		cons_prints(", ");
		cons_printDecimal(new_rect->bottom);
		cons_prints(", ");
		cons_printDecimal(new_rect->right);
		cons_prints("\n");
	}
	
	scans(10, inbuf);
*/
	return outrect;	
}

void drawOccluded(window* win, Rect* baserect, List* splitrect_list) {
	
	if(!splitrect_list) 
		return;
	
	int split_count = 0;
	int total_count = 1;
	int working_total = 0;
	List* out_rects;
	Rect* working_rects = (Rect*)0;
	int i, j, k;
    Rect *new_rect, *rect, *split_rect, *out_rect;
		
#ifdef RECT_TEST
	
    //Clear everything 
    setColor(RGB(255, 255, 255));
    setCursor(0, 0);
    fillRect(root_window->context->width, root_window->context->height);
    
    //Draw the base window
    setColor(RGB(0, 0, 255));
    setCursor(baserect->left, baserect->top);
    drawRect(baserect->right - baserect->left, baserect->bottom - baserect->top);
    
    //Draw the overlapping windows
    List_for_each(splitrect_list, rect, Rect*) {
        
        setColor(RGB(255, 0, 0));
        setCursor(rect->left, rect->top);
        drawRect(rect->right - rect->left, rect->bottom - rect->top);
    }
    
#endif //RECT_TEST
    
    //If there's nothing occluding us, just render the bitmap and get out of here
    if(!splitrect_list->count) {
		
        drawBmpRect(win, baserect);
        return;
    }
    
    //prints("[WYG] Allocating space for output rectangles\n");
	out_rects = List_new();
    
    if(!out_rects) {
        
        //prints("[WYG] Couldn't allocate space for output rect list\n");
		return;
    }
    
    rect = Rect_new(baserect->top, baserect->left, baserect->bottom, baserect->right);
    
    if(!rect) {
        
        //prints("[WYG] Couldn't allocate space for temp rectangle\n");
		List_delete(out_rects, Rect_deleter);
        return;
    }
    
    if(!List_add(out_rects, (void*)rect)) {
        
        //prints("[WYG] Couldn't insert out rect into list\n");
		free((void*)rect);
		List_delete(out_rects, Rect_deleter);
        return;
    }
		        
	//For each splitting rect, split each rect in out_rects, delete the rectangle that was split, and add the resultant split rectangles
	List_for_each(splitrect_list, split_rect, Rect*) {
		
		List_for_each(out_rects, out_rect, Rect*) {
            
			if((split_rect->left <= out_rect->right &&
			   split_rect->right >= out_rect->left &&
			   split_rect->top <= out_rect->bottom && 
			   split_rect->bottom >= out_rect->top)) {
			    
                List* clip_list = splitRect(out_rect, split_rect);

#ifdef RECT_TEST
			            
                //for(k = 0; k < split_count; k++)
                    //printf("split %u, %u, %u, %u\n", split_rects[k].top, split_rects[k].left, split_rects[k].bottom, split_rects[k].right);

#endif //RECT_TEST
            
			    if(!clip_list) {
					
					List_delete(out_rects, Rect_deleter);
					return;
				}
			
				//If nothing was returned, we actually want to clip a rectangle in its entirety
				if(!clip_list->count) {
					
					List_remove(out_rects, (void*)out_rect, Rect_deleter);
					
					//If we deleted the last output rectangle, we are completely 
					//occluded and can return early
					if(out_rects->count == 0) {
						
						List_delete(clip_list, Rect_deleter);
						List_delete(out_rects, Rect_deleter);
						return;
					}
					
					//Otherwise, go back to the top of the loop and test the next out_rect
					continue;
				}
							
				//Replace the rectangle that got split with the first result rectangle 
				rect = (Rect*)List_get_at(clip_list, 0);
				out_rect->top = rect->top;
				out_rect->left = rect->left;
				out_rect->bottom = rect->bottom;
				out_rect->right = rect->right;
				
				//Append the rest of the result rectangles to the output collection
				List_for_each_skip(clip_list, rect, Rect*, 1) {
                    
                    new_rect = Rect_new(rect->top, rect->left, rect->bottom, rect->right);
                    
                    if(!new_rect) {
						List_delete(clip_list, Rect_deleter);
						List_delete(out_rects, Rect_deleter);	
                        return;
					}
                    
                    if(!List_add(out_rects, (void*)new_rect)){
						
						free((void*)new_rect);
						List_delete(clip_list, Rect_deleter);
						List_delete(out_rects, Rect_deleter);	
                        return;
					}
				}
				
				//Free the space that was used for the split 
                                List_delete(clip_list, Rect_deleter);
				
				//Restart the list 
				List_rewind(out_rects);
			} 
		}
		
//		cons_prints("Moving on to next splitter\n");
	}
	
//	cons_prints("Drawing sub-rects of window #");
//    cons_printDecimal(win->handle);
//	cons_putc('\n');
    List_for_each(out_rects, out_rect, Rect*) {

#ifdef RECT_TEST    
        //printf("%u, %u, %u, %u\n", out_rects[k].top, out_rects[k].left, out_rects[k].bottom, out_rects[k].right);
#endif //RECT_TEST
        
		
/*
		cons_prints("    (");
		cons_printDecimal(out_rect->top);
		cons_prints(", ");
		cons_printDecimal(out_rect->left);
		cons_prints(", ");
		cons_printDecimal(out_rect->bottom);
		cons_prints(", ");
		cons_printDecimal(out_rect->right);
		cons_prints(")\n");               
*/  
      drawBmpRect(win, out_rect);     
    }
	
//	scans(10, inbuf);
	
	List_delete(out_rects, Rect_deleter);
}

window* newWindow(unsigned int width, unsigned int height, unsigned char flags, unsigned int pid) {
    
	static int next_handle = 1; 
    window *new_window, *temp_window;
    unsigned int i, bufsz;
    	
    if(!(new_window = (window*)malloc(sizeof(window)))) {
        
        return 0;
    }
        
    new_window->active = 1;
	new_window->pid = pid;
    new_window->flags = flags;
    new_window->x = 0;
    new_window->y = 0;
    new_window->w = width;
    new_window->h = height;
    new_window->title = (unsigned char*)0;
    new_window->frame_needs_redraw = 1;
    
    //Create a drawing context for the new window
    if(!(new_window->context = newBitmap(new_window->w, new_window->h))) {
        
        free((void*)new_window);
        return (window*)0;
    } 
    
    bufsz = new_window->w * new_window->h;
    
    //Clear new window to white
    for(i = 0; i < bufsz; i++)
        new_window->context->data[i] = RGB(255, 255 ,255);
        
    new_window->handle = next_handle++;

    if(mouse_window)
        List_pop(window_list, (void*)mouse_window);

    //De-activate the old active window
    if(temp_window = (window*)List_get_at(window_list, window_list->count - 1)) {

        temp_window->active = 0;
    } 
	
	
    if(!List_add(window_list, (void*)new_window)){
		
	freeBitmap(new_window->context);
        free((void*)new_window);
		
 	//re-activate the old active window
	if(temp_window)
	    temp_window->active = 1;
        
	return (window*)0;	
    }
    
    //Give the new window its initial decoration
    if(!(new_window->flags & WIN_UNDECORATED))
        drawFrame(new_window);
	
    drawWindow(new_window, 0);
	
    //Update the titlebar on the old active window 
    if(temp_window)
 	drawTitlebar(temp_window, 1);
	
    if(mouse_window) {

        List_add(window_list, mouse_window);
        drawWindow(mouse_window, 0);
    }

    //cmd_printDecimal(new_window->handle);
    //pchar('\n');
    return new_window;
}

unsigned int newWindowHandle(unsigned int width, unsigned int height, unsigned char flags, unsigned int pid) {
	
	window* ret_window = newWindow(width, height, flags, pid);
	
	if(ret_window)
	    return ret_window->handle;
	
	return 0;
}

window* getWindowByHandle(unsigned int handle) {
    
	window* out_window;
	    
    List_for_each(window_list, out_window, window*) {
        
		if(out_window->handle == handle)
		    return out_window;
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
void updateOverlapped(Rect* window_bounds, window* avoid_window) {
    
    int i = 0;
    Rect comp_rect, draw_rect; 
    window* cur_window;
	
    //prints("[WYG] Looking for previously overlapped windows\n");
        
    for(i = 0; i < window_list->count; i++) {

        cur_window = (window*)List_get_at(window_list, i);
        
	if(!cur_window || cur_window == avoid_window)
	    continue;
		
        comp_rect.top = cur_window->y;
        comp_rect.left = cur_window->x;
        comp_rect.bottom = comp_rect.top + cur_window->h - 1;
        comp_rect.right = comp_rect.left + cur_window->w - 1;
        
        if((cur_window->flags & WIN_VISIBLE) && 
	   window_bounds->left <= comp_rect.right &&
           window_bounds->right >= comp_rect.left &&
           window_bounds->top <= comp_rect.bottom && 
           window_bounds->bottom >= comp_rect.top) {
            
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
            
            cur_window->context->top = draw_rect.top - cur_window->y;
            cur_window->context->left = draw_rect.left - cur_window->x;       
            cur_window->context->bottom = draw_rect.bottom - cur_window->y;
            cur_window->context->right = draw_rect.right - cur_window->x;        
            drawWindow(cur_window, 1);
        }
    }
}

void markWindowVisible(window* dest_window, unsigned char is_visible);

void changeWindowPosition(window* dest_window, unsigned short new_x, unsigned short new_y) {
    
    Rect overlap_rect;
            
    //If a window is moved, we must ensure that it is the active window 
	markWindowVisible(dest_window, 1);
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
        
		//Should update this so that we don't redraw stuff that's going to
		//be under the window's new location because we're going to draw
		//over that when we draw the window at the new location anyhow     
        updateOverlapped(&overlap_rect, dest_window); //Redraw all of the siblings that this window was covering up
        
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
       
    changeWindowPosition(dest_window, new_x, new_y);
}

void installWindow(unsigned int child_handle, unsigned int parent_handle) {
    
	//Right now, we removed all of the parent-child relationships in the window object,
	//so this doesn't really do anything. In the future, we should probably do 
	//something with it, though
	
	/*
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
	*/   
}

void markWindowVisible(window* dest_window, unsigned char is_visible) {
    
    unsigned char was_visible;
    Rect overlap_rect;
    
    was_visible = dest_window->flags & WIN_VISIBLE;
    
	if(!!was_visible && !!is_visible)
	    return;

    if(is_visible) {
               
        dest_window->flags |= WIN_VISIBLE;
	drawWindow(dest_window, 0);
    } else {
        
        dest_window->flags &= ~((unsigned char)WIN_VISIBLE);
	overlap_rect.top = dest_window->y;
        overlap_rect.left = dest_window->x;
        overlap_rect.bottom = overlap_rect.top + dest_window->h - 1;
        overlap_rect.right = overlap_rect.left + dest_window->w - 1;
        updateOverlapped(&overlap_rect, dest_window); //Redraw all of the siblings that this window was covering up
    }
    	
    return;
}

void markHandleVisible(unsigned int handle, unsigned char is_visible) {
    
    window* dest_window = getWindowByHandle(handle);
    
    if(!dest_window) {
     
         //prints("[WYG] Couldn't find window to mark it visible\n");   
        return;
    }
    
    markWindowVisible(dest_window, is_visible);
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
    
    drawTitlebar(dest_window, 1);
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

void drawTitlebar(window* cur_window, int do_refresh) {
    
    unsigned char* s;    
    unsigned int tb_color, text_color;
    Rect old_ctx_rect;
    
	if(cur_window->flags & WIN_UNDECORATED)
	    return;
	    
    //Titlebar
    if(cur_window->active)
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
        
        if(cur_window->active)
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
    
	if(do_refresh) {
	
		old_ctx_rect.top = cur_window->context->top;
		old_ctx_rect.left = cur_window->context->left;
		old_ctx_rect.bottom = cur_window->context->bottom;
		old_ctx_rect.right = cur_window->context->right;
		
		cur_window->context->top = 4;
		cur_window->context->left = 4;
		cur_window->context->bottom = 23;
		cur_window->context->right = cur_window->w - 26;
		
		drawWindow(cur_window, 1);
		
		cur_window->context->top = old_ctx_rect.top;
		cur_window->context->left = old_ctx_rect.left;
		cur_window->context->bottom = old_ctx_rect.bottom;
		cur_window->context->right = old_ctx_rect.right;
	}
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
    bmpDrawPanel(cur_window->context, 3, 27, cur_window->w - 6, cur_window->h - 30, RGB(238, 203, 137), 1, 1);
    
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
    
    drawTitlebar(cur_window, 0);
    
    cur_window->frame_needs_redraw = 0;
}

List* getOverlappingWindows(int lowest_z_level, Rect* baserect) {

    List* rect_list = List_new();
	
	if(!rect_list) {
		
		return (List*)0;
	}
	    
	Rect* new_rect;
	window* cur_window;
    
	List_for_each_skip(window_list, cur_window, window*, lowest_z_level) {
		
		//Count the window only if it overlaps
		if((cur_window->flags & WIN_VISIBLE) && 
                   cur_window->context->mask_color == 0 &&
                   cur_window->x <= baserect->right &&
		   (cur_window->x + cur_window->context->width - 1) >= baserect->left &&
		   cur_window->y <= baserect->bottom && 
		   (cur_window->y + cur_window->context->height - 1) >= baserect->top) {
		
		        if(!(new_rect = Rect_new(cur_window->y, cur_window->x, (cur_window->y + cur_window->context->height - 1), (cur_window->x + cur_window->context->width - 1)))) {
					
					List_delete(rect_list, Rect_deleter);
					return (List*)0;
				}
				
				if(!List_add(rect_list, new_rect)) {
					
					free((void*)new_rect);
					List_delete(rect_list, Rect_deleter);
					return (List*)0;
				}
		} 
	}
        
    return rect_list;
}

//To be used in raise window code
void drawWindowIntersects(window* cur_window, unsigned char use_current_blit) {

}

void drawWindow(window* cur_window, unsigned char use_current_blit) {
     
    unsigned int rect_count;
    List* splitrect_list;
    Rect winrect;
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
        
        
		if(!(splitrect_list = getOverlappingWindows(List_get_index(window_list, (void*)cur_window) + 1, &winrect))) { //build the rects
		
			return;
		}        
        		
        drawOccluded(cur_window, &winrect, splitrect_list);   
        //prints("[WYG] Finished doing occluded draw\n");    
        
        //getch();
        
        List_delete(splitrect_list, Rect_deleter);       
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
    
    //Draw the window, assume we want to use the blit window set up by the client   
    drawWindow(dest_window, 1);
}

void raiseWindow(window* dest_window) {
    
    window* old_active;
            
    //Can't raise the root window, mouse window, a null window pointer or if there's nothing but the root and
    //mouse in the window list
    if(dest_window == root_window || dest_window == mouse_window || !dest_window || window_list->count <= 2)
        return;
        
    //Get the previously active window (will be one deeper than the mouse, hence
    //-2 instead of normal -1)
    old_active = (window*)List_get_at(window_list, window_list->count - 2);
    
    //If we were already active we don't need to do anything else 
    if(old_active == dest_window)
        return;

    //extract the current window from its position in the list and
    //re-insert it at the end, making sure to pop and restore the
    //mouse window as well to keep it always on top
    if(!List_pop(window_list, (void*)mouse_window))
        return;

    if(!List_pop(window_list, (void*)dest_window))
        return;

    if(!List_add(window_list, (void*)dest_window))
        return;

    if(!List_add(window_list, (void*)mouse_window))
        return;
		
    //Update the titlebar on the old and new active windows 
    old_active->active = 0;
    dest_window->active = 1;
    drawTitlebar(old_active, 1);
    drawTitlebar(dest_window, 1);
	
    //If the window isn't visible, it will need to be in order to be
    //raised, otherwise we just redraw (would be more efficient in
    //the future to just redraw those portions that were occluded
    //prior to raising)
    if(!(dest_window->flags &= WIN_VISIBLE))
        markWindowVisible(dest_window, 1);
    else
        drawWindow(dest_window, 0);
}

void raiseHandle(unsigned int handle) {

    //Can't raise the root or mouse windows
    if(handle == 1 || handle == 2)
        return;
    
    window* dest_window = getWindowByHandle(handle);

    if(!dest_window) {
     
         //prints("[WYG] Couldn't find the window to be raised\n");   
        return;
    }
    
    raiseWindow(dest_window);
}

void window_deleter(void* item) {
	
	window* win = (window*)item;
	
	//Free the context
    freeBitmap((void*)win->context);

#ifndef HARNESS_TEST    
    //Free the title (if we ever decide to error on unsuccessful frees, this could be an issue for static or undefined titles)
    if(win->title)
	free((void*)win->title);
#endif //HARNESS_TEST
    
    //And finally free ourself 
    free((void*)win);
}

void destroy(window* dest_window) {

    window *cur_child, *next, *active_window;
    int i;
            
    //Start by hiding the window 
    markWindowVisible(dest_window, 0);
    List_remove(window_list, (void*)dest_window, window_deleter);
    active_window = (window*)List_get_at(window_list, window_list->count - 1);
	
	if(!active_window)
	    return;
		
	if(!active_window->active) {
		
		active_window->active;
		drawTitlebar(active_window, 1);
	}
}

void destroyHandle(unsigned int handle) {
    
    window* dest_window = getWindowByHandle(handle);
    
    if(!dest_window) 
        return;
        
    destroy(dest_window);
}

unsigned int cons_x, cons_y;
unsigned int cons_max_c, cons_max_l;

/*
void cons_init() {
	
	cons_x = 0;
	
	if(inited)
		cons_y = 1;
	else 
	    cons_y = 0;
		
	cons_max_c = root_window->w / 8;
	cons_max_l = (root_window->h / 12) - cons_y; 
}

void cons_putc(char c) {
	
	if(cons_y >= cons_max_l)
	    return;
	
	if(c == '\n') {
	    
		cons_x = 0;
		cons_y++;
	    return;
	}
	
	drawCharacter(c, cons_x * 8, cons_y * 12, RGB(0, 0, 0)); 
	
	cons_x++;
	
	if(cons_x >= cons_max_c) {
		cons_x = 0;
		cons_y++;
	}
}

void cons_prints(char* s) {
	
	while(*s) 
	    cons_putc(*(s++));
}

void cons_printDecimal(unsigned int dword) {

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
        cons_putc(digit[j] + '0');
}

void cons_printHexByte(unsigned char byte) {

    cons_putc(digitToHex((byte & 0xF0)>>4));
    cons_putc(digitToHex(byte & 0xF));
}


void cons_printHexWord(unsigned short wd) {

    cons_printHexByte((unsigned char)((wd & 0xFF00)>>8));
    cons_printHexByte((unsigned char)(wd & 0xFF));
}


void cons_printHexDword(unsigned int dword) {

    cons_printHexWord((unsigned short)((dword & 0xFFFF0000)>>16));
    cons_printHexWord((unsigned short)(dword & 0xFFFF));
}
*/

void window_printer(void* value) {
	

	window* win = (window*)value;

/*    
EM_ASM_({	
	console.log("    window " +
	            $0            +
	            " @ 0x"       +
	            $1
        );
}, win->handle, (unsigned int)win);
*/
}

void exceptionHandler(void) {

/*	
	cons_init();
	cons_prints("Operating system raised an exception\n");
	cons_prints("There were ");
	cons_printDecimal(window_list->count);
	cons_prints(" windows installed:\n");
	List_print(window_list, window_printer);
*/
	while(1);
}

unsigned char mouse_down = 0;
window* drag_window = (window*)0;
int drag_x, drag_y;

void putMouse(int x, int y, unsigned char buttons) {

    int i;
    window* cur_window;

    changeWindowPosition(mouse_window, x, y);
    
    if(buttons) {

        if(!mouse_down) {

            mouse_down = 1;
        
            for(i = window_list->count - 2; i > 0; i--) {

                cur_window = (window*)List_get_at(window_list, i);   

                if(!cur_window || cur_window == root_window || cur_window == mouse_window) 
                    continue;

                if(x >= cur_window->x &&
                   x < cur_window->x + cur_window->w &&
                   y >= cur_window->y &&
                   y < cur_window->y + cur_window->h) {

                    drag_x = x - cur_window->x;
                    drag_y = y - cur_window->y;
                    drag_window = cur_window;
                    break;
                }
            }
        }
    } else {
       
        mouse_down = 0;
        drag_window = (window*)0;
    }

    if(mouse_down && drag_window) {
   
         changeWindowPosition(drag_window, x - drag_x, y - drag_y);
    }
}

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

    if(mouse_x > root_window->w - 20)
        mouse_x = root_window->w - 20;

    if(mouse_y < 0)
        mouse_y = 0;
    
    if(mouse_y > root_window->h - 20)
        mouse_y = root_window->h - 20;

    putMouse((unsigned short)mouse_x, (unsigned short)mouse_y, buttons);
}

#define MOUSE_WIDTH 11
#define MOUSE_HEIGHT 18
#define MOUSE_BUFSZ (MOUSE_WIDTH * MOUSE_HEIGHT)
#define CA 0x0
#define CB 0xFFFFFF
#define CD 0xFF000000

unsigned int mouse_img[MOUSE_BUFSZ] = {
CA, CD, CD, CD, CD, CD, CD, CD, CD, CD, CD,
CA, CA, CD, CD, CD, CD, CD, CD, CD, CD, CD,
CA, CB, CA, CD, CD, CD, CD, CD, CD, CD, CD,
CA, CB, CB, CA, CD, CD, CD, CD, CD, CD, CD,
CA, CB, CB, CB, CA, CD, CD ,CD, CD, CD, CD,
CA, CB, CB, CB, CB, CA, CD, CD, CD, CD, CD,
CA, CB, CB, CB, CB, CB, CA, CD, CD, CD, CD,
CA, CB, CB, CB, CB, CB, CB, CA, CD, CD, CD,
CA, CB, CB, CB, CB, CB, CB, CB, CA, CD, CD,
CA, CB, CB, CB, CB, CB, CB, CB, CB, CA, CD,
CA, CB, CB, CB, CB, CB, CB, CB, CB, CB, CA,
CA, CA, CA, CA, CB, CB, CB, CA, CA, CA, CA,
CD, CD, CD, CD, CA, CB, CB, CA, CD, CD, CD,
CD, CD, CD, CD, CA, CB, CB, CA, CD, CD, CD,
CD, CD, CD, CD, CD, CA, CB, CB, CA, CD, CD,
CD, CD, CD, CD, CD, CA, CB, CB, CA, CD, CD,
CD, CD, CD, CD, CD, CD, CA, CB, CA, CD, CD,
CD, CD, CD, CD, CD, CD, CD, CA, CA, CD, CD 
};

void malloc_start(void) {

    alloc_ta = getElapsedMs();
}

void malloc_end(void) {

    alloc_time += getElapsedMs() - alloc_ta;
}

void screenThread() {

    while(1) {

/*
        if(back_buffer == 0xB1C000) {
 
            setCursor(0, 0);
            setColor(RGB(255, 0, 0));
            drawStr("BAD POINTER");
            prints("BAD POINTER");
            while(1);
        }
*/
        setCursor(0, 0);
        drawBitmap(back_buffer);
        sleep(20);
    }
}

void statusThread() {

    while(1) {

        draw_time = 0;
        alloc_time = 0;
        sleep(5000);
        printDecimal(alloc_time);
        prints("a/");
        printDecimal(draw_time);
        prints("d/5000ms\n");
    }
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
    unsigned int src_pid;
    unsigned char* instr;
    unsigned int strlen;

#ifndef HARNESS_TEST

/*
    enable_debug(malloc_start, malloc_end);

    if(!startThread())
        statusThread();
*/
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

#endif //HARNESS_TEST

    prints("[WYG] Getting graphics service... \n");
    if(!initGfx()) {
        
        //prints("\n[WYG] failed to get the GFX server.\n");
        postMessage(REGISTRAR_PID, REG_DEREGISTER, SVC_WYG);
        postMessage(parent_pid, 0, 0); //Tell the parent we're done registering
        terminate();
    }

#ifdef HARNESS_TEST

    num = 1;

#else

    //Prompt user for a screen mode
    showModes();
    prints("mode: ");
    scans(10, inbuf);
    num = inbuf[0] > '9' ? inbuf[0] - 'A' + 10 : inbuf[0] - '0';

#endif //HARNESS_TEST

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
    
    back_buffer = newBitmap(mode->width, mode->height);

    //Would realistically be on a vsync interrupt
    if(!startThread())
        screenThread();

	//cmd_init(mode->width, mode->height);
	
    if(!(window_list = List_new())) {
        
        prints("[WYG] Couldn't allocate window list.\n");
        postMessage(REGISTRAR_PID, REG_DEREGISTER, SVC_WYG);
        postMessage(parent_pid, 0, 0); //Tell the parent we're done registering
        terminate();
    }
    
    //Init the root window (aka the desktop)
    root_window = newWindow(mode->width, mode->height, WIN_UNDECORATED | WIN_FIXEDSIZE | WIN_VISIBLE, 0);
	
    //Create a drawing context for the root window
    if(!root_window) {
        
        //prints("[WYG] Could not allocate a context for the root window.\n");
        //Need to do a list free here for the window_list
        postMessage(REGISTRAR_PID, REG_DEREGISTER, SVC_WYG);
        postMessage(parent_pid, 0, 0); //Tell the parent we're done registering
        terminate();
    } 

    //Set up the initial mouse position
    mouse_x = root_window->w / 2 - 1;
    mouse_y = root_window->h / 2 - 1;

    //Create the mouse window
    mouse_window = newWindow(MOUSE_WIDTH, MOUSE_HEIGHT, WIN_UNDECORATED | WIN_FIXEDSIZE | WIN_VISIBLE, 0);

    //Fail if the mouse couldn't be created
    if(!mouse_window) {
        
        postMessage(REGISTRAR_PID, REG_DEREGISTER, SVC_WYG);
        postMessage(parent_pid, 0, 0);
        terminate();
    }

    changeWindowPosition(mouse_window, mouse_x, mouse_y);

    postMessage(parent_pid, 0, 1); //Tell the parent we're done registering

    //Paint the initial scene
    for(i = 0; i < root_window->w * root_window->h; i++)
        root_window->context->data[i] = RGB(11, 162, 193);

    //Paint the mouse cursor
    mouse_window->context->mask_color = CD;

    for(i = 0; i < MOUSE_BUFSZ; i++)
        mouse_window->context->data[i] = mouse_img[i];
    
    drawWindow(root_window, 0);
    drawWindow(mouse_window, 0);
	        
    //Start debug console
    //init(root_window.w, 48);

#ifdef HARNESS_TEST

    //enter the testing code
    testMain();
    endGfx();
    return;

#else 
    
	//cons_init();
	
    //Now we can start the main message loop 
	//cmd_prints("Wyg started");
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
                postMessage(src_pid, WYG_CREATE_WINDOW, (unsigned int)newWindowHandle((temp_msg.payload & 0xFFF00000) >> 20, (temp_msg.payload & 0xFFF00) >> 8, temp_msg.payload & 0xFF, src_pid));
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
                moveMouse(temp_msg.payload);
            break;

            default:
            break;
        }
    }
    
#endif //HARNESS_TEST
    
}
