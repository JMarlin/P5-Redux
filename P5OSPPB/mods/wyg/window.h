#ifndef WINDOW_H
#define WINDOW_H 

#ifdef __cplusplus
extern "C" {
#endif

#include "context.h"
#include "inttypes.h"
#include "object.h"

//================| Window Class Declaration |================//

//Feel free to play with this 'theme'
#define WIN_BGCOLOR     RGB(238, 203, 137) //A generic grey
#define WIN_TITLECOLOR  RGB(182, 0, 0) //A nice subtle blue
#define WIN_TITLECOLOR_INACTIVE RGB(238, 203, 137) //A darker shade 
#define WIN_TEXTCOLOR RGB(255, 255, 255)
#define WIN_TEXTCOLOR_INACTIVE RGB(138, 103, 37)
#define WIN_BORDERCOLOR 0xFF000000 //Straight-up black
#define WIN_TITLEHEIGHT 28 
#define WIN_BORDERWIDTH 4

//Some flags to define our window behavior
#define WIN_NODECORATION 0x1
#define WIN_NORAISE 0x2
#define WIN_BODYDRAG 0x4
#define WIN_HIDDEN 0x8

//Forward struct declaration for function type declarations
struct Window_struct;

//Callback function type declarations
typedef void (*WindowPaintHandler)(struct Window_struct*);
typedef void (*WindowMousedownHandler)(struct Window_struct*, int, int);
typedef void (*WindowMouseupHandler)(struct Window_struct*, int, int);
typedef void (*WindowMouseoverHandler)(struct Window_struct*);
typedef void (*WindowMouseoutHandler)(struct Window_struct*);
typedef void (*WindowMousemoveHandler)(struct Window_struct*, int, int);
typedef void (*WindowMouseclickHandler)(struct Window_struct*, int, int);
typedef void (*WindowMoveHandler)(struct Window_struct*, int, int);

typedef struct Window_struct {
    Object object;  
    struct Window_struct* parent;
    unsigned int id;
    int16_t x;
    int16_t y;
    uint16_t width;
    uint16_t height;
    uint16_t flags;
    Context* context;
    struct Window_struct* drag_child;
    struct Window_struct* active_child;
    struct Window_struct* over_child;
    List* children;
    uint16_t drag_off_x;
    uint16_t drag_off_y;
    uint8_t last_button_state;
    uint8_t click_cycle;
    WindowPaintHandler paint_function;
    WindowMousedownHandler mousedown_function;
    WindowMouseupHandler mouseup_function;
    WindowMouseoverHandler mouseover_function;
    WindowMouseoutHandler mouseout_function;
    WindowMousemoveHandler mousemove_function;
    WindowMouseclickHandler mouseclick_function;
    WindowMoveHandler move_function;
    char* title;
} Window;

//Methods
Window* Window_new(int16_t x, int16_t y, uint16_t width,
                   uint16_t height, uint16_t flags, Context* context);
int Window_init(Window* window, int16_t x, int16_t y, uint16_t width,
                uint16_t height, uint16_t flags, Context* context);
void Window_mousedown(Window* window, int x, int y);
void Window_mouseup(Window* window, int x, int y);
void Window_mouseover(Window* window);
void Window_mouseout(Window* window);
void Window_mousemove(Window* window, int x, int y);
void Window_mouseclick(Window* window, int x, int y);
int Window_screen_x(Window* window);
int Window_screen_y(Window* window);                   
void Window_paint(Window* window, List* dirty_regions, uint8_t paint_children);
void Window_process_mouse(Window* window, uint16_t mouse_x,
                          uint16_t mouse_y, uint8_t mouse_buttons);
void Window_paint_handler(Window* window);
void Window_mousedown_handler(Window* window, int x, int y);
List* Window_get_windows_above(Window* parent, Window* child);
List* Window_get_windows_below(Window* parent, Window* child);
void Window_raise(Window* window, uint8_t do_draw);
void Window_move(Window* window, int new_x, int new_y);
void Window_move_function(Window* window, int new_x, int new_y);
void Window_hide(Window* window);
void Window_show(Window* window);
Window* Window_create_window(Window* window, int16_t x, int16_t y,  
                             uint16_t width, int16_t height, uint16_t flags);
void Window_update_context(Window* window, Context* context);                             
void Window_insert_child(Window* window, Window* child);   
void Window_invalidate(Window* window, int top, int left, int bottom, int right);
void Window_set_title(Window* window, char* new_title);                       
void Window_append_title(Window* window, char* additional_chars);                      
void Window_delete_function(Object* window_object);
void Window_resize(Window* window, int w, int h);
void print_window(Window* window);

#ifdef __cplusplus
}
#endif

#endif //WINDOW_H
