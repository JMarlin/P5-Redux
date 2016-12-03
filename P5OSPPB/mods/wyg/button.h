#ifndef BUTTON_H
#define BUTTON_H

#ifdef __cplusplus
extern "C" {
#endif

#include "window.h"

#define BUT_DEPRESSED 0x1
#define BUT_OVER      0x2

struct Button_struct;

typedef void (*ButtonMousedownHandler)(struct Button_struct*, int, int);

typedef struct Button_struct {
    Window window;
    uint8_t depressed;
} Button;

Button* Button_new(int x, int y, int w, int h);
void Button_mousedown_handler(Window* button_window, int x, int y);
void Button_mouseup_handler(Window* button_window, int x, int y);
void Button_mouseout_handler(Window* button_window);
void Button_mouseover_handler(Window* button_window);
void Button_paint(Window* button_window);
void Button_delete_handler(void* button_object);

#ifdef __cplusplus
}
#endif

#endif //BUTTON_H