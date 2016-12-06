#include "inttypes.h"
#include "../include/memory.h"
#include "../include/wyg.h"
#include "window.h"
#include "styleutils.h"


//================| Window Class Implementation |================//

//Here's a quick, crappy pseudo-RNG since you probably don't have one
uint8_t pseudo_rand_8() {

    static uint16_t seed = 0;
    return (uint8_t)(seed = (12657 * seed + 12345) % 256);
}

void print_window(Window* window) {
/*
    printf("\n-----------------\nNEW WINDOW\n-----------------\n");
    printf("Object deleter #: %p\n", (void*)window->object.delete_function);
    printf("Parent pointer: 0x%08X\n", (int)window->parent);
    printf("Window ID: %i\n", window->id);
    printf("Window position: (%i, %i)\n", window->x, window->y);
    printf("Window dimensions: %i x %i\n", window->width, window->height);
    printf("Window flags: 0x%04X\n", window->flags);
    printf("Context pointer: 0x%08X\n", (int)window->context);
    printf("Drag child pointer: 0x%08X\n", (int)window->drag_child);
    printf("Active child pointer: 0x%08X\n", (int)window->active_child);
    printf("Over child pointer: 0x%08X\n", (int)window->over_child);
    printf("Child list pointer: 0x%08X\n", (int)window->children);
    printf("Drag offsets: (%i, %i)\n", window->drag_off_x, window->drag_off_y);
    printf("Last button state: 0x%02X\n", window->last_button_state);
    printf("Click cycle: %i\n", window->click_cycle);
    printf("Paint function #: %p\n", (void*)window->paint_function);
    printf("Mousedown function #: %p\n", (void*)window->mousedown_function);
    printf("Mouseup function #: %p\n", (void*)window->mouseup_function);
    printf("Mouseover function #: %p\n", (void*)window->mouseover_function);
    printf("Mouseout function #: %p\n", (void*)window->mouseout_function);
    printf("Mousemove function #: %p\n", (void*)window->mousemove_function);
    printf("Mouseclick function #: %p\n", (void*)window->mouseclick_function);
    printf("Move function #: %p\n", (void*)window->move_function);
    if(window->title)
        printf("Window title: %s\n", window->title);
    else
        printf("Window title:\n");
*/
}

//Window constructor
Window* Window_new(int16_t x, int16_t y, uint16_t width,
                   uint16_t height, uint16_t flags, Context* context) {

    //Try to allocate space for a new WindowObj and fail through if malloc fails
    Window* window;
    if(!(window = (Window*)malloc(sizeof(Window))))
        return window;

    //Attempt to initialize the new window
    if(!Window_init(window, x, y, width, height, flags, context)) {
    
        free(window);
        return (Window*)0;
    }

    return window;
}

//Seperate object allocation from initialization so we can implement
//our inheritance scheme
int Window_init(Window* window, int16_t x, int16_t y, uint16_t width,
                uint16_t height, uint16_t flags, Context* context) {

    static unsigned int handle_source = 0;

    Object_init((Object*)window, Window_delete_function);

    //Moved over here from the desktop 
    //Create child list or clean up and fail
    if(!(window->children = List_new()))
        return 0;

    //Assign the property values
    window->id = ++handle_source;
    window->x = x;
    window->y = y;
    window->width = width;
    window->height = height;
    window->context = context ? Context_new_from(context) : context;
    window->flags = flags;
    window->parent = (Window*)0;
    window->drag_child = (Window*)0;
    window->drag_off_x = 0;
    window->drag_off_y = 0;
    window->last_button_state = 0;
    window->click_cycle = 0;
    window->paint_function = Window_paint_handler;
    window->mousedown_function = (WindowMousedownHandler)0;
    window->mouseup_function = (WindowMouseupHandler)0;
    window->mouseover_function = (WindowMouseoverHandler)0;
    window->mouseout_function = (WindowMouseoutHandler)0;
    window->mousemove_function = (WindowMousemoveHandler)0;
    window->mouseclick_function = (WindowMouseclickHandler)0;
    window->move_function = Window_move_function;
    window->active_child = (Window*)0;
    window->over_child = (Window*)0;
    window->title = (char*)0;
    window->pid = 0;

    return 1;
}

void Window_mousedown(Window* window, int x, int y) {

    if(window->click_cycle != 2) 
        return;

    window->click_cycle = 3;

    if(window->mousedown_function)
        window->mousedown_function(window, x, y);
}

void Window_mouseup(Window* window, int x, int y) {

    if(window->mouseup_function)
        window->mouseup_function(window, x, y);

    if(window->click_cycle == 3)
        Window_mouseclick(window, x, y);

    window->click_cycle = 1;
}

void Window_mouseover(Window* window) {

    window->click_cycle = 1;

    if(window->mouseover_function)
        window->mouseover_function(window);
}

void Window_mouseout(Window* window) {

    int old_click_cycle = window->click_cycle;

    window->click_cycle = 0;

    if(old_click_cycle == 3)
        Window_mouseup(window, 0, 0);

    if(window->over_child) {

        Window_mouseout(window->over_child);
        window->over_child = (Window*)0;
    }

    if(window->mouseout_function)
        window->mouseout_function(window);    
}

void Window_mousemove(Window* window, int x, int y) {

    if(window->mousemove_function)
        window->mousemove_function(window, x, y);
}

void Window_mouseclick(Window* window, int x, int y) {

    if(window->mouseclick_function)
        window->mouseclick_function(window, x, y);
}

//Recursively get the absolute on-screen x-coordinate of this window
int Window_screen_x(Window* window) {

    if(window->parent)
        return window->x + Window_screen_x(window->parent);
    
    return window->x;
}

//Recursively get the absolute on-screen y-coordinate of this window
int Window_screen_y(Window* window) {

    if(window->parent)
        return window->y + Window_screen_y(window->parent);
    
    return window->y;
}

void Window_draw_border(Window* window) {

    uint32_t tb_color;
    int screen_x = Window_screen_x(window);
    int screen_y = Window_screen_y(window);
    
    //Outer border
    draw_panel(window->context, screen_x, screen_y, window->width,
               window->height, WIN_BGCOLOR, 1, 0);
    
    //Title border
    draw_panel(window->context, screen_x+3, screen_y+3, window->width - 6,
               22, WIN_BGCOLOR, 1, 1);
    
    //Body border
    draw_panel(window->context, screen_x+3, screen_y+27, window->width - 6,
               window->height - 30, WIN_BGCOLOR, 1, 1);
    
    //Left frame
    Context_fill_rect(window->context, screen_x+1, screen_y+1, 2, 
                      window->height - 2, WIN_BGCOLOR); 
    
    //Right frame
    Context_fill_rect(window->context, screen_x + window->width - 3,
                      screen_y + 1, 2, window->height - 2, WIN_BGCOLOR); 
    
    //Top frame
    Context_fill_rect(window->context, screen_x + 3, screen_y + 1,
                      window->width - 6, 2, WIN_BGCOLOR); 
    
    //Mid frame
    Context_fill_rect(window->context, screen_x + 3, screen_y + 25,
                      window->width - 6, 2, WIN_BGCOLOR); 
    
    //Bottom frame
    Context_fill_rect(window->context, screen_x + 3, screen_y + window->height - 3,
                      window->width - 6, 2, WIN_BGCOLOR); 
        
    //Button
    draw_panel(window->context, screen_x + window->width - 24, screen_y + 4,
               20, 20, WIN_BGCOLOR, 1, 0);
    Context_fill_rect(window->context, screen_x + window->width - 23,
                      screen_y + 5, 18, 18, WIN_BGCOLOR); 
    
    //Titlebar
    if(window->parent->active_child == window)
        tb_color = RGB(182, 0, 0);
    else 
        tb_color = RGB(238, 203, 137);
    
    Context_fill_rect(window->context, screen_x + 4, screen_y + 4,
                      window->width - 28, 20, tb_color);

    //Draw the window title
    if(window->title)
        Context_draw_text(window->context, window->title, (WIN_TITLEHEIGHT / 2) + screen_x - 6,
                          (WIN_TITLEHEIGHT / 2) + screen_y - 6,
                          window->parent->active_child == window ? 
                              WIN_TEXTCOLOR : WIN_TEXTCOLOR_INACTIVE);

}

//Apply clipping for window bounds without subtracting child window rects
void Window_apply_bound_clipping(Window* window, Context* context, int in_recursion, List* dirty_regions) {

    Rect *temp_rect, *current_dirty_rect, *clone_dirty_rect;
    int screen_x, screen_y, i;
    List* clip_windows;
    Window* clipping_window;

    //Can't do this without a context
    if(!context)
        return;

    //Build the visibility rectangle for this window
    //If the window is decorated and we're recursing, we want to limit
    //the window's drawable area to the area inside the window decoration.
    //If we're not recursing, however, it means we're about to paint 
    //ourself and therefore we want to wait until we've finished painting
    //the window border to shrink the clipping area 
    screen_x = Window_screen_x(window);
    screen_y = Window_screen_y(window);
    
    if((!(window->flags & WIN_NODECORATION)) && in_recursion) {

        //Limit client drawable area 
        screen_x += WIN_BORDERWIDTH;
        screen_y += WIN_TITLEHEIGHT;
        temp_rect = Rect_new(screen_y, screen_x,
                             screen_y + window->height - WIN_TITLEHEIGHT - WIN_BORDERWIDTH - 1, 
                             screen_x + window->width - (2*WIN_BORDERWIDTH) - 1);
    } else {

        temp_rect = Rect_new(screen_y, screen_x, screen_y + window->height - 1, 
                             screen_x + window->width - 1);
    }

    //If there's no parent (meaning we're at the top of the window tree)
    //then we just add our rectangle and exit
    //Here's our change: If we were passed a dirty region list, we first
    //clone those dirty rects into the clipping region and then intersect
    //the top-level window bounds against it so that we're limited to the
    //dirty region from the outset
    if(!window->parent) {

        if(dirty_regions) {

            //Clone the dirty regions and put them into the clipping list
            for(i = 0; i < dirty_regions->count; i++) {
            
                //Clone
                current_dirty_rect = (Rect*)List_get_at(dirty_regions, i);
                clone_dirty_rect = Rect_new(current_dirty_rect->top,
                                            current_dirty_rect->left,
                                            current_dirty_rect->bottom,
                                            current_dirty_rect->right);
                
                //Add
                Context_add_clip_rect(context, clone_dirty_rect);
            }

            //Finally, intersect this top level window against them
            Context_intersect_clip_rect(context, temp_rect);

        } else {

            Context_add_clip_rect(context, temp_rect);
        }

        return;
    }

    //Otherwise, we first reduce our clipping area to the visibility area of our parent
    Window_apply_bound_clipping(window->parent, context, 1, dirty_regions);

    //Now that we've reduced our clipping area to our parent's clipping area, we can
    //intersect our own bounds rectangle to get our main visible area  
    Context_intersect_clip_rect(context, temp_rect);

    //And finally, we subtract the rectangles of any siblings that are occluding us 
    clip_windows = Window_get_windows_above(window->parent, window);

    while(clip_windows->count) {
        
        clipping_window = (Window*)List_remove_at(clip_windows, 0);

        //Get a rectangle from the window, subtract it from the clipping 
        //region, and dispose of it
        screen_x = Window_screen_x(clipping_window);
        screen_y = Window_screen_y(clipping_window);

        temp_rect = Rect_new(screen_y, screen_x,
                             screen_y + clipping_window->height - 1,
                             screen_x + clipping_window->width - 1);
        Context_subtract_clip_rect(context, temp_rect);
        Object_delete((Object*)(Object*)temp_rect);
    }

    //Dispose of the used-up list 
    Object_delete((Object*)clip_windows);
}

void Window_update_title(Window* window) {

    int screen_x, screen_y;

    if(!window->context || 
       (window->flags & WIN_HIDDEN) ||
       (window->flags & WIN_NODECORATION))
        return;

    //Start by limiting painting to the window's visible area
    Window_apply_bound_clipping(window, window->context, 0, (List*)0);

    //Draw border
    Window_draw_border(window);

    Context_clear_clip_rects(window->context);
}

//Request a repaint of a certain region of a window
void Window_invalidate(Window* window, int top, int left, int bottom, int right) {

    List* dirty_regions;
    Rect* dirty_rect;

    //This function takes coordinates in terms of window coordinates
    //So we need to convert them to screen space 
    int origin_x = Window_screen_x(window);
    int origin_y = Window_screen_y(window);
    top += origin_y;
    bottom += origin_y;
    left += origin_x;
    right += origin_x;
    
    //Attempt to create a new dirty rect list 
    if(!(dirty_regions = List_new()))
        return;

    if(!(dirty_rect = Rect_new(top, left, bottom, right))) {

        Object_delete((Object*)dirty_regions);
        return;
    }

    if(!List_add(dirty_regions, (Object*)dirty_rect)) {

        Object_delete((Object*)dirty_regions);
        return;
    }

    Window_paint(window, dirty_regions, 0);

    //Clean up the dirty rect list
    Object_delete((Object*)dirty_regions);
}

//Another override-redirect function
void Window_paint(Window* window, List* dirty_regions, uint8_t paint_children) {

    int i, j, screen_x, screen_y, child_screen_x, child_screen_y;
    Window* current_child;
    Rect* temp_rect;

    //Can't paint without a context
    if(!window->context || (window->flags & WIN_HIDDEN))
        return;

    //Start by limiting painting to the window's visible area
    Window_apply_bound_clipping(window, window->context, 0, dirty_regions);

    //Set the context translation
    screen_x = Window_screen_x(window);
    screen_y = Window_screen_y(window);

    //If we have window decorations turned on, draw them and then further
    //limit the clipping area to the inner drawable area of the window 
    if(!(window->flags & WIN_NODECORATION)) {

        //Draw border
        Window_draw_border(window);

        //Limit client drawable area 
        screen_x += WIN_BORDERWIDTH;
        screen_y += WIN_TITLEHEIGHT;
        temp_rect = Rect_new(screen_y, screen_x,
                             screen_y + window->height - WIN_TITLEHEIGHT - WIN_BORDERWIDTH - 1, 
                             screen_x + window->width - (2*WIN_BORDERWIDTH) - 1);
        Context_intersect_clip_rect(window->context, temp_rect);
    }

    //Then subtract the screen rectangles of any children 
    //NOTE: We don't do this in Window_apply_bound_clipping because, due to 
    //its recursive nature, it would cause the screen rectangles of all of 
    //our parent's children to be subtracted from the clipping area -- which
    //would eliminate this window. 
    for(i = 0; i < window->children->count; i++) {

        current_child = (Window*)List_get_at(window->children, i);

        if(current_child->flags & WIN_HIDDEN)
            continue;

        child_screen_x = Window_screen_x(current_child);
        child_screen_y = Window_screen_y(current_child);

        temp_rect = Rect_new(child_screen_y, child_screen_x,
                             child_screen_y + current_child->height - 1,
                             child_screen_x + current_child->width - 1);
        Context_subtract_clip_rect(window->context, temp_rect);
        Object_delete((Object*)temp_rect);
    }

    //Finally, with all the clipping set up, we can set the context's 0,0 to the top-left corner
    //of the window's drawable area, and call the window's final paint function 
    window->context->translate_x = screen_x;
    window->context->translate_y = screen_y;
    
    //TODO: Add a message interface for requesting subscription to paint 
    //and other events. If the external process requests a subscription
    //to painting, that means that it is taking over all repaint duties
    //and so, if that subscription flag is set for the current window, we
    //should skip any draw code in this function and instead send the event 
    //message to the owning process. This should actually probably be done
    //in the Window_paint method so that subscription to painting will 
    //override any and all painting functions that a particular widget class 
    //may have replaced this default function with 
    //In lieu of that, for now we're just going to always ship off that message 

    //Should check for both PID and subscription flag in the future
    if(window->pid) {

        postMessage(window->pid, WYG_EVENT, WYG_EVENT_REPAINT);

        //We need another message method that allows the client to tell us that a 
        //draw is complete and therefore to clear the translation and clipping, but
        //for now we're just going to be lazy (which will lead to weird results)
    } else {

        //Do an internal repaint
        window->paint_function(window);

        //If we did an internal repaint, we're not waiting on anything so we can clear everything right away
        //Now that we're done drawing this window, we can clear the changes we made to the context
        Context_clear_clip_rects(window->context);
        window->context->translate_x = 0;
        window->context->translate_y = 0;
    }
    
    //Even though we're no longer having all mouse events cause a redraw from the desktop
    //down, we still need to call paint on our children in the case that we were called with
    //a dirty region list since each window needs to be responsible for recursively checking
    //if its children were dirtied 
    if(!paint_children)
        return;

    for(i = 0; i < window->children->count; i++) {

        current_child = (Window*)List_get_at(window->children, i);

        if(dirty_regions) {

            //Check to see if the child is affected by any of the
            //dirty region rectangles
            for(j = 0; j < dirty_regions->count; j++) {
            
                temp_rect = (Rect*)List_get_at(dirty_regions, j);
                
                screen_x = Window_screen_x(current_child);
                screen_y = Window_screen_y(current_child);

                if(temp_rect->left <= (screen_x + current_child->width - 1) &&
                   temp_rect->right >= screen_x &&
                   temp_rect->top <= (screen_y + current_child->height - 1) &&
                   temp_rect->bottom >= screen_y)
                    break;
            }

            //Skip drawing this child if no intersection was found
            if(j == dirty_regions->count)
                continue;
        }

        //Otherwise, recursively request the child to redraw its dirty areas
        Window_paint(current_child, dirty_regions, 1);
    }
}

//This is the default paint method for a new window
void Window_paint_handler(Window* window) {

    //Fill in the window background
    Context_fill_rect(window->context, 0, 0,
                      window->width, window->height, WIN_BGCOLOR);
}

//Used to get a list of windows overlapping the passed window
List* Window_get_windows_above(Window* parent, Window* child) {

    int i;
    Window* current_window;
    List* return_list;

    //Attempt to allocate the output list
    if(!(return_list = List_new()))
        return return_list;

    //We just need to get a list of all items in the
    //child list at higher indexes than the passed window
    //We start by finding the passed child in the list
    for(i = 0; i < parent->children->count; i++)
        if(child == (Window*)List_get_at(parent->children, i))
            break;

    //Now we just need to add the remaining items in the list
    //to the output (IF they overlap, of course)
    //NOTE: As a bonus, this will also automatically fall through
    //if the window wasn't found
    for(i++; i < parent->children->count; i++) {

        current_window = (Window*)List_get_at(parent->children, i);

        if(current_window->flags & WIN_HIDDEN)
            continue;

        //Our good old rectangle intersection logic
        if(current_window->x <= (child->x + child->width - 1) &&
		   (current_window->x + current_window->width - 1) >= child->x &&
		   current_window->y <= (child->y + child->height - 1) &&
		   (current_window->y + current_window->height - 1) >= child->y)
            List_add(return_list, (Object*)current_window); //Insert the overlapping window
    }

    return return_list; 
}

//Used to get a list of windows which the passed window overlaps
//Same exact thing as get_windows_above, but goes backwards through
//the list. Could probably be made a little less redundant if you really wanted
List* Window_get_windows_below(Window* parent, Window* child) {

    int i;
    Window* current_window;
    List* return_list;

    //Attempt to allocate the output list
    if(!(return_list = List_new()))
        return return_list;

    //We just need to get a list of all items in the
    //child list at higher indexes than the passed window
    //We start by finding the passed child in the list
    for(i = parent->children->count - 1; i > -1; i--)
        if(child == (Window*)List_get_at(parent->children, i))
            break;

    //Now we just need to add the remaining items in the list
    //to the output (IF they overlap, of course)
    //NOTE: As a bonus, this will also automatically fall through
    //if the window wasn't found
    for(i--; i > -1; i--) {

        current_window = (Window*)List_get_at(parent->children, i);

        if(current_window->flags & WIN_HIDDEN)
            continue;

        //Our good old rectangle intersection logic
        if(current_window->x <= (child->x + child->width - 1) &&
		   (current_window->x + current_window->width - 1) >= child->x &&
		   current_window->y <= (child->y + child->height - 1) &&
		   (current_window->y + current_window->height - 1) >= child->y)
            List_add(return_list, (Object*)current_window); //Insert the overlapping window
    }

    return return_list; 
}

//Breaking 
void Window_raise(Window* window, uint8_t do_draw) {

    int i;
    Window* parent;
    Window* last_active = (Window*)0;

    if(window->flags & WIN_NORAISE)
        return;

    if(!window->parent)
        return;

    parent = window->parent;

    if(parent->active_child == window)
        return;

    last_active = parent->active_child;

    //Find the child in the list
    for(i = 0; i < parent->children->count; i++)
        if((Window*)List_get_at(parent->children, i) == window)
            break;

    List_remove_at(parent->children, i); //Pull window out of list
    List_add(parent->children, (void*)window); //Insert at the top

    //Make it active 
    parent->active_child = window;

    //Do a redraw if it was requested
    if(!do_draw)
        return;

    Window_paint(window, (List*)0, 1);

    //Make sure the old active window gets an updated title color 
    if(last_active) 
        Window_update_title(last_active);
}

void Window_move(Window* window, int new_x, int new_y) {

    if(window->move_function)
        window->move_function(window, new_x, new_y);
    else
        Window_move_function(window, new_x, new_y);
}

//We're wrapping this guy so that we can handle any needed redraw
void Window_move_function(Window* window, int new_x, int new_y) {

    int i;
    int old_x = window->x;
    int old_y = window->y;
    Rect new_window_rect;
    List *replacement_list, *dirty_list, *dirty_windows;

    //To make life a little bit easier, we'll make the not-unreasonable 
    //rule that if a window is moved, it must become the top-most window
    Window_raise(window, 0); //Raise it, but don't repaint it yet

    //We'll hijack our dirty rect collection from our existing clipping operations
    //So, first we'll get the visible regions of the original window position
    Window_apply_bound_clipping(window, window->context, 0, (List*)0);

    //Temporarily update the window position
    window->x = new_x;
    window->y = new_y;

    //Calculate the new bounds
    new_window_rect.top = Window_screen_y(window);
    new_window_rect.left = Window_screen_x(window);
    new_window_rect.bottom = new_window_rect.top + window->height - 1;
    new_window_rect.right = new_window_rect.left + window->width - 1;

    //Reset the window position
    window->x = old_x;
    window->y = old_y;

    //Now, we'll get the *actual* dirty area by subtracting the new location of
    //the window 
    Context_subtract_clip_rect(window->context, &new_window_rect);

    //Now that the context clipping tools made the list of dirty rects for us,
    //we can go ahead and steal the list it made for our own purposes
    //(yes, it would be cleaner to spin off our boolean rect functions so that
    //they can be used both here and by the clipping region tools, but I ain't 
    //got time for that junk)
    if(!(replacement_list = List_new())) {

        Context_clear_clip_rects(window->context);
        return;
    }

    dirty_list = window->context->clip_rects;
    window->context->clip_rects = replacement_list;

    //Now, let's get all of the siblings that we overlap before the move
    dirty_windows = Window_get_windows_below(window->parent, window);

    window->x = new_x;
    window->y = new_y;

    //And we'll repaint all of them using the dirty rects
    //(removing them from the list as we go for convenience)
    while(dirty_windows->count)
        Window_paint((Window*)List_remove_at(dirty_windows, 0), dirty_list, 1);

    //The one thing that might still be dirty is the parent we're inside of
    Window_paint(window->parent, dirty_list, 0);

    //We're done with the lists, so we can dump them
    Object_delete((Object*)dirty_list);
    Object_delete((Object*)dirty_windows);

    //With the dirtied siblings redrawn, we can do the final update of 
    //the window location and paint it at that new position
    Window_paint(window, (List*)0, 1);
}

//Interface between windowing system and mouse device
void Window_process_mouse(Window* window, uint16_t mouse_x,
                          uint16_t mouse_y, uint8_t mouse_buttons) {

    int i, inner_x1, inner_y1, inner_x2, inner_y2;
    Window* child;

    if(window->drag_child) {

        if(mouse_buttons) {

            //Changed to use 
            Window_move(window->drag_child, mouse_x - window->drag_off_x,
                        mouse_y - window->drag_off_y);
            return;
        } else {

            window->drag_child = (Window*)0;
        }
    }

    //If we had a button depressed, then we need to see if the mouse was
    //over any of the child windows
    //We go front-to-back in terms of the window stack for free occlusion
    for(i = window->children->count - 1; i >= 0; i--) {

        child = (Window*)List_get_at(window->children, i);

        //If mouse isn't window bounds, we can't possibly be interacting with it 
        if(!(mouse_x >= child->x && mouse_x < (child->x + child->width) &&
           mouse_y >= child->y && mouse_y < (child->y + child->height)) || 
           (child->flags & WIN_HIDDEN)) 
            continue;

        //Do mouseover and mouseout events 
        if(child != window->over_child) {

            if(window->over_child)
                Window_mouseout(window->over_child);
            else
                Window_mouseout(window);

            window->over_child = child;
            Window_mouseover(window->over_child);
        }            

        //Now we'll check to see if we're dragging a titlebar
        if(mouse_buttons && !window->last_button_state) {

            //Let's adjust things so that a raise happens whenever we click inside a 
            //child, to be more consistent with most other GUIs
            Window_raise(child, 1);

            //See if the window has bodydrag enabled or 
            //See if the mouse position lies within the bounds of the current titlebar
            //We check the decoration flag since we can't drag a window without a titlebar
            if((child->flags & WIN_BODYDRAG) ||  (
               !(child->flags & WIN_BODYDRAG) && !(child->flags & WIN_NODECORATION) &&
               mouse_y >= child->y && mouse_y < (child->y + WIN_TITLEHEIGHT)
               )) {

                //We'll also set this window as the window being dragged
                //until such a time as the mouse is released
                window->drag_off_x = mouse_x - child->x;
                window->drag_off_y = mouse_y - child->y;
                window->drag_child = child;
            }
        }
        
        break;
    }

    //Do any not-over-a-child handling
    if(i < 0) {

        //If we were previously over a child, handle a mouseout event on it and clear the pointer
        if(window->over_child) {

            Window_mouseout(window->over_child);
            window->over_child = (Window*)0;
            
            //We reentered the parent from a child, so fire a mouseover on the parent 
            Window_mouseover(window);
        }

        //If we didn't find a target in the search, then we ourselves are the target of any clicks
        if(mouse_buttons && !window->last_button_state) 
            Window_mousedown(window, mouse_x, mouse_y);

        if(!mouse_buttons && window->last_button_state)
            Window_mouseup(window, mouse_x, mouse_y);

        Window_mousemove(window, mouse_x, mouse_y);
    } else {

        //Found a target, so forward the mouse event to that window and quit looking
        Window_process_mouse(child, mouse_x - child->x, mouse_y - child->y, mouse_buttons); 

        //Cancel any body drag if the mouse was found to be over a child in the dragged child 
        if((child->flags & WIN_BODYDRAG) && (window->drag_child == child) && !!child->over_child)
            window->drag_child = (Window*)0;
    }

    //Update the stored mouse button state to match the current state 
    window->last_button_state = mouse_buttons;
    if(window->click_cycle == 1)
        window->click_cycle = 2;
}

void Window_update_context(Window* window, Context* context) {

    int i;
    Context* old_context = window->context;

    window->context = context ? Context_new_from(context) : context;

    for(i = 0; i < window->children->count; i++)
        Window_update_context((Window*)List_get_at(window->children, i), context);

    if(old_context)
        Object_delete((Object*)old_context);
}

//Quick wrapper for shoving a new entry into the child list
void Window_insert_child(Window* window, Window* child) {

    child->parent = window;
    List_add(window->children, (Object*)child);   
    Window_update_context(child, window->context);
    //Window_raise(child, 1); //Don't necessarily want to show the child
                              //as soon as it's installed
}

void Window_remove_child(Window* window, Window* child) {

    int i;

    //Find the child's location in the parent list
    for(i = 0; i < window->children->count; i++)
        if(child == (Window*)List_get_at(window->children, i))
            break;
    
    //If it wasn't found, exit early
    if(i == window->children->count)
        return;

    //Disassociate the child and parent
    Window_hide(child);
    List_remove_at(window->children, i);
    child->parent = (Window*)0;
    Window_update_context(child, (Context*)0);   
}

//A method to automatically create a new window in the provided parent window
Window* Window_create_window(Window* window, int16_t x, int16_t y,  
                             uint16_t width, int16_t height, uint16_t flags) {

    //Attempt to create the window instance
    Window* new_window;
    if(!(new_window = Window_new(x, y, width, height, flags, window->context)))
        return new_window;

    //Attempt to add the window to the end of the parent's children list
    //If we fail, make sure to clean up all of our allocations so far 
    if(!List_add(window->children, (void*)new_window)) {

        Object_delete((Object*)new_window);
        return (Window*)0;
    }

    //Set the new child's parent 
    new_window->parent = window;

    Window_raise(new_window, 1);

    return new_window;
}

//Assign a string to the title of the window
void Window_set_title(Window* window, char* new_title) {

    int len, i;

    //Make sure to free any preexisting title 
    if(window->title) {

        for(len = 0; window->title[len]; len++);
        free(window->title);
    }

    //We don't have strlen, so we're doing this manually
    for(len = 0; new_title[len]; len++);

    //Try to allocate new memory to clone the string
    //(+1 because of the trailing zero in a c-string)
    if(!(window->title = (char*)malloc((len + 1) * sizeof(char))))
        return;

    //Clone the passed string into the window's title
    //Including terminating zero
    for(i = 0; i <= len; i++)
        window->title[i] = new_title[i];

    //Make sure the change is reflected on-screen
    if(window->flags & WIN_NODECORATION)
        Window_invalidate(window, 0, 0, window->height - 1, window->width - 1);
    else
        Window_update_title(window);
}

//Add the characters from the passed string to the end of the window title
void Window_append_title(Window* window, char* additional_chars) {

    char* new_string;
    int original_length, additional_length, i;

    //Set the title if there isn't already one
    if(!window->title) {

        Window_set_title(window, additional_chars);
        return;
    }

    //Get the length of the original string
    for(original_length = 0; window->title[original_length]; original_length++);

    //Get the length of the new string
    for(additional_length = 0; additional_chars[additional_length]; additional_length++);

    //Try to malloc a new string of the needed size
    if(!(new_string = (char*)malloc(sizeof(char) * (original_length + additional_length + 1)))) {
        return;
    }

    //Copy the base string into the new string
    for(i = 0; window->title[i]; i++)
        new_string[i] = window->title[i];

    //Copy the appended chars into the new string
    for(i = 0; additional_chars[i]; i++)
        new_string[original_length + i] = additional_chars[i];

    //Add the final zero char
    new_string[original_length + i] = 0;

    //And swap the string pointers
    free(window->title);
    window->title = new_string;

    //Make sure the change is reflected on-screen
    if(window->flags & WIN_NODECORATION)
        Window_invalidate(window, 0, 0, window->height - 1, window->width - 1);
    else
        Window_update_title(window); 
}

void Window_hide(Window* window) {

    List* dirty_list;
    Rect* dirty_rect;

    if(!window->parent || (window->flags & WIN_HIDDEN))
        return;

    window->flags |= WIN_HIDDEN;
    
    //Build a dirty rect list for the mouse area
    if(!(dirty_list = List_new()))
        return;

    if(!(dirty_rect = Rect_new(Window_screen_y(window), Window_screen_x(window), 
                               Window_screen_y(window) + window->height - 1,
                               Window_screen_x(window) + window->width - 1))) {

        Object_delete((Object*)dirty_list);
        return;
    }

    List_add(dirty_list, (Object*)dirty_rect);

    //Do a dirty update for the desktop, which will, in turn, do a 
    //dirty update for all affected child windows
    Window_paint(window->parent, dirty_list, 1); 

    Object_delete((Object*)dirty_list);
}

void Window_show(Window* window) {

    if(!(window->flags & WIN_HIDDEN))
        return;

    window->flags &= ~WIN_HIDDEN;

    Window_paint(window, (List*)0, 1);
}

void Window_delete_function(Object* window_object) {

    int i;
    Window *window = (Window*)window_object;

    if(!window_object)
        return;

    Window_hide(window);

    Object_delete((Object*)window->children);

    if(window->parent) {

        for(i = 0;
            i < window->parent->children->count && 
            (Window*)List_get_at(window->parent->children, i) != window; 
            i++);

        if(i < window->parent->children->count)
            List_remove_at(window->parent->children, i);

        if(window->parent->active_child == window) {

            if(window->parent->children->count) {

                window->parent->active_child =
                    (Window*)List_get_at(window->parent->children,
                                         window->parent->children->count - 1);

                Window_update_title(window->parent->active_child);
            } else {

                window->parent->active_child = (Window*)0;
            }
        }

        if(window->parent->over_child == window)
            window->parent->over_child = (Window*)0;
        
        if(window->parent->drag_child == window)
            window->parent->drag_child = (Window*)0;
    }

    Object_delete((Object*)window->context);
    free(window);
}

void Window_resize(Window* window, int w, int h) {

    window->width = w;
    window->height = h;

    Window_paint(window, (List*)0, 1);
}