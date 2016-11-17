#include "button.h"
#include "styleutils.h"

Button* Button_new(int x, int y, int w, int h) {

    //Normal allocation and initialization
    //Like a Desktop, this is just a special kind of window 
    Button* button;
    if(!(button = (Button*)malloc(sizeof(Button))))
        return button;

    if(!Window_init((Window*)button, x, y, w, h, WIN_NODECORATION | WIN_NORAISE, (Context*)0)) {

        free(button);
        return (Button*)0;
    }

    //Override default window callbacks
    button->window.paint_function = Button_paint;
    button->window.mousedown_function = Button_mousedown_handler;
    button->window.mouseup_function = Button_mouseup_handler;
    button->window.mouseout_function = Button_mouseout_handler;
    button->window.mouseover_function = Button_mouseover_handler;
    button->window.mouseclick_function = (WindowMouseclickHandler)0;
    
    //And clear the toggle value
    button->depressed = 0;

    return button;
}

void Button_paint(Window* button_window) {

    int title_len;
    Button* button = (Button*)button_window;

    uint32_t bgcolor = button->depressed == BUT_OVER ?
                       RGB(RVAL(WIN_BGCOLOR) + 0x10, GVAL(WIN_BGCOLOR) + 0x10, BVAL(WIN_BGCOLOR) + 0x10) :
                       WIN_BGCOLOR;

    draw_panel(button_window->context, 0, 0, button_window->width,
               button_window->height, bgcolor, 1, button->depressed == BUT_DEPRESSED);
    Context_fill_rect(button_window->context, 1, 1, button_window->width - 2,
                      button_window->height - 2, bgcolor);   

    //Get the title length
    for(title_len = 0; button_window->title[title_len]; title_len++);

    //Convert it into pixels
    title_len *= 8;

    //Draw the title centered within the button
    if(button_window->title)
        Context_draw_text(button_window->context, button_window->title,
                          (button_window->width / 2) - (title_len / 2),
                          (button_window->height / 2) - 6,
                          WIN_BORDERCOLOR);                                          
}

//This just sets and resets the toggle
void Button_mousedown_handler(Window* button_window, int x, int y) {

    Button* button = (Button*)button_window;

    button->depressed = BUT_DEPRESSED;

    //Since the button has visibly changed state, we need to invalidate the
    //area that needs updating
    Window_invalidate((Window*)button, 0, 0,
                      button->window.height - 1, button->window.width - 1);
}

void Button_mouseup_handler(Window* button_window, int x, int y) {

    Button* button = (Button*)button_window;

    button->depressed = BUT_OVER;

    //Since the button has visibly changed state, we need to invalidate the
    //area that needs updating
    Window_invalidate((Window*)button, 0, 0,
                      button->window.height - 1, button->window.width - 1);
}

void Button_mouseout_handler(Window* button_window) {

    Button* button = (Button*)button_window;

    button->depressed = 0;

    //Since the button has visibly changed state, we need to invalidate the
    //area that needs updating
    Window_invalidate((Window*)button, 0, 0,
                      button->window.height - 1, button->window.width - 1);
}

void Button_mouseover_handler(Window* button_window) {

    Button* button = (Button*)button_window;

    button->depressed = BUT_OVER;

    //Since the button has visibly changed state, we need to invalidate the
    //area that needs updating
    Window_invalidate((Window*)button, 0, 0,
                      button->window.height - 1, button->window.width - 1);
}