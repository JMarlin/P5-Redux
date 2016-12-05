#include "wygcommands.h"

//Forward declaration of widget class constructors
Window* WYG_window_constructor(unsigned int flags);
Window* WYG_button_constructor(unsigned int flags);
Window* WYG_textbox_constructor(unsigned int flags);

//Global data for window creation purposes
const unsigned char widget_class_count = 3;
const Window* (*widget_class_constructor)(unsigned int) widget_class_constructors[] = {
    WYG_window_constructor,
    WYG_button_constructor,
    WYG_textbox_constructor
};

//Get a pointer to the window with the specified ID
Window* WYG_get_window_from_id(Window* parent, unsigned int window_id) {

    int i;
    Window* current_child;
    
    //Search for the child among the parent window's children
    for(i = 0; i < parent->children->count; i++) {

        //Get the next child in the list and see if it's the one we want
        current_child = (Window*)List_get_at(parent->children, i);
        
        //If so, return it and exit
        if(current_child->id == window_id)
            return current_child;

        //If the current child wasn't the one we're looking for, recursively
        //check the current child's children for the window in question
        current_child = WYG_get_window_from_id(current_child, window_id);

        //If the window was found in the current child window's tree, return
        //it and exit
        if(current_child)
            return current_child;
    }

    //If we got this far, the requested ID wasn't found in the parent or any
    //of its child windows
    return (Window*)0;
}

//Generate a new window and temporarily install it into the passed desktop, 
//returning the ID of the new window 
unsigned int WYG_create_window(Desktop* desktop, unsigned int flags) {

    Window* window; 
    unsigned char widget_type;

    //Get the widget type from the flags
    widget_type = (unsigned char)((flags >> 24) & 0xFF);
    
    //Fail if not a valid widget type
    if(widget_type >= widget_class_count)
        return 0;

    //Pass the flags, without the type portion, to the constructor of the specified type
    window = widget_class_constructors[widget_type](flags & 0xFFFFFF);

    //If the constructor failed, return a failure
    if(!window)
        return 0;

    //Otherwise, temporarily stash the new window in the passed desktop and hand back 
    //the new window id
    Window_insert_child((Window*)desktop, window);

    return window->id;
}

//Get the id of the drawing context that belongs to the window with the provided ID
unsigned int WYG_get_window_context_id(Desktop* desktop, unsigned int window_id) {

    Window* window;

    //Try to find the window in the window tree of the passed desktop
    window = WYG_get_window_from_id(desktop, window_id);

    //If we couldn't find the window, fail
    if(!window)
        return 0;

    //otherwise, return the context ID 
    if(window->context)
        return window->context->id;
    else
        return 0;
}

//Find the specified window and return its dimensions packed for a message
unsigned int WYG_get_window_dimensions(Desktop* desktop, unsigned int window_id) {

    Window* window;

    //Try to find the window in the window tree of the passed desktop
    window = WYG_get_window_from_id(desktop, window_id);

    //If we couldn't find the window, fail
    if(!window)
        return 0;

    //Otherwise, return the packed dimensions
    return ((window->width & 0xFFFF) << 16) | (window->height & 0xFFFF); 
}

//Find the specified window and return its x,y loaction packed for a message
unsigned int WYG_get_window_location(Desktop* desktop, unsigned int window_id) {

    Window* window;

    //Try to find the window in the window tree of the passed desktop
    window = WYG_get_window_from_id(desktop, window_id);

    //If we couldn't find the window, fail
    if(!window)
        return 0;

    //Otherwise, return the packed dimensions
    return ((window->x & 0xFFFF) << 16) | (window->y & 0xFFFF); 
}

//Find the specified window and update its x,y position as given in a packed uint32
void WYG_move_window(Desktop* desktop, unsigned int window_id, unsigned int position_data) {

    Window* window;

    //Try to find the window in the window tree of the passed desktop
    window = WYG_get_window_from_id(desktop, window_id);

    //If we couldn't find the window, fail
    if(!window)
        return;

    //Otherwise, move the window
    Window_move(window, (position_data >> 16) & 0xFFFF, position_data & 0xFFFF);
}

//Find the specified window, remove it from its parent if it has one set, then find the 
//specified parent window and install it into that new parent 
void WYG_install_window(Desktop* desktop, unsigned int child_id, unsigned int parent_id) {

    Window* child;
    Window* parent;

    //Try to find the windows in the window tree of the passed desktop
    window = WYG_get_window_from_id(desktop, child_id);
    parent = WYG_get_window_from_id(desktop, parent_id);

    //If we couldn't find the windows, fail
    if(!child || !parent)
        return;

    //If the child belongs to a parent, uninstall it 
    if(child->parent)
        Window_remove_child(child->parent, child);

    //Install the child into the new parent 
    Window_insert_child(parent, child);
}

//Find the specified window and display it
void WYG_show_window(Desktop* desktop, unsigned int window_id) {

    Window* window;

    //Try to find the window in the window tree of the passed desktop
    window = WYG_get_window_from_id(desktop, window_id);

    //If we couldn't find the window, fail
    if(!window)
        return;

    //Otherwise, show it 
    Window_show(window);
}

//Find the specified window and bring it to the top of its parent's window stack
void WYG_raise_window(Desktop* desktop, unsigned int window_id) {

    Window* window;

    //Try to find the window in the window tree of the passed desktop
    window = WYG_get_window_from_id(desktop, window_id);

    //If we couldn't find the window, fail
    if(!window)
        return;

    //Raise the window 
    Window_raise(window, 1);
}

//Find the specified window and initiate a redraw of the whole thing
//NOTE: We support invalidation as limited by a dirty rect and will need to implement that in 
//      the future
void WYG_invalidate_window(Desktop* desktop, unsigned int window_id) {

    Window* window;

    //Try to find the window in the window tree of the passed desktop
    window = WYG_get_window_from_id(desktop, window_id);

    //If we couldn't find the window, fail
    if(!window)
        return;

    //Perform the invalidation
    Window_invalidate(window, window->y, window->x,
                      window->y + window->height - 1,
                      window->x + window->width - 1);
}

//Find the specified window and update its title with the passed value
void WYG_set_window_title(Desktop* desktop, unsigned int window_id, char* new_title) {

    Window* window;

    //Try to find the window in the window tree of the passed desktop
    window = WYG_get_window_from_id(desktop, window_id);

    //If we couldn't find the window, fail
    if(!window)
        return;

    //Set the title
    Window_set_title(window, new_title);
}

//Find the specified window and delete it 
void WYG_destroy_window(Desktop* desktop, unsigned int window_id) {

    Window* window;

    //Try to find the window in the window tree of the passed desktop
    window = WYG_get_window_from_id(desktop, window_id);

    //If we couldn't find the window, fail
    if(!window)
        return;
    
    //Delete the window
    Object_delete((Object*)window);
}

//Return a packed version of WYG's window border widths
unsigned int WYG_get_frame_dims() {

    top left bottom right

    return ((WIN_TITLEHEIGHT & 0xFF) << 24) | ((WIN_BORDERWIDTH & 0xFF) << 16) |
           ((WIN_BORDERWIDTH & 0xFF) << 8) | (WIN_BORDERWIDTH & 0xFF);
}
