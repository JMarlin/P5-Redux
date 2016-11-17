#ifndef CONTEXT_H
#define CONTEXT_H

#ifdef __cplusplus
extern "C" {
#endif

struct Context_struct;

#include "inttypes.h"
#include "list.h"
#include "rect.h"
#include "object.h"

//================| Context Class Declaration |================//

//A structure for holding information about a framebuffer
typedef struct Context_struct {  
    Object object;
    unsigned int id;
    uint32_t* buffer; //A pointer to our framebuffer
    uint16_t width; //The dimensions of the framebuffer
    uint16_t height; 
    int translate_x; //Our new translation values
    int translate_y;
    List* clip_rects;
    uint8_t clipping_on;
} Context;

//Methods
Context* Context_new(uint16_t width, uint16_t height, uint32_t* buffer);
Context* Context_new_from(Context* source_context);
void Context_delete_function(Object* context_object);
void Context_fill_rect(Context* context, int x, int y,  
                       unsigned int width, unsigned int height, uint32_t color);
void Context_horizontal_line(Context* context, int x, int y,
                             unsigned int length, uint32_t color);
void Context_vertical_line(Context* context, int x, int y,
                           unsigned int length, uint32_t color);                                                   
void Context_draw_rect(Context* context, int x, int y,
                       unsigned int width, unsigned int height, uint32_t color);
void Context_intersect_clip_rect(Context* context, Rect* rect);                       
void Context_subtract_clip_rect(Context* context, Rect* subtracted_rect);                       
void Context_add_clip_rect(Context* context, Rect* rect);
void Context_clear_clip_rects(Context* context);
void Context_draw_text(Context* context, char* string, int x, int y, uint32_t color);

#ifdef __cplusplus
}
#endif

#endif //CONTEXT_H
