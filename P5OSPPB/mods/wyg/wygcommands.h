#ifndef WYGCOMMANDS_H
#define WYGCOMMANDS_H

#include "desktop.h"

unsigned int WYG_create_window(Desktop* desktop, unsigned int flags);
unsigned int WYG_get_window_context_id(Desktop* desktop, unsigned int window_id);
unsigned int WYG_get_window_dimensions(Desktop* desktop, unsigned int window_id);
unsigned int WYG_get_window_location(Desktop* desktop, unsigned int window_id);
void WYG_move_window(Desktop* desktop, unsigned int window_id, unsigned int position_data);
void WYG_resize_window(Desktop* desktop, unsigned int window_id, unsigned int size_data);
void WYG_install_window(Desktop* desktop, unsigned int child_id, unsigned int parent_id);
void WYG_show_window(Desktop* desktop, unsigned int window_id);
void WYG_raise_window(Desktop* desktop, unsigned int window_id);
void WYG_invalidate_window(Desktop* desktop, unsigned int window_id);
void WYG_set_window_title(Desktop* desktop, unsigned int window_id, char* new_title);
void WYG_destroy_window(Desktop* desktop, unsigned int window_id);
unsigned int WYG_get_frame_dims();

#endif //WYGCOMMANDS_H
