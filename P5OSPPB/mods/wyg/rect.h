#ifndef RECT_H
#define RECT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "object.h"
#include "list.h"
#include "../include/memory.h"

//================| Rect Class Declaration |================//

typedef struct Rect_struct {
    Object object;
    int top;
    int left;
    int bottom;
    int right;
} Rect;

//Method declarations
Rect* Rect_new(int top, int left, int bottom, int right);
List* Rect_split(Rect* subject_rect, Rect* cutting_rect);
Rect* Rect_intersect(Rect* rect_a, Rect* rect_b);

#ifdef __cplusplus
}
#endif

#endif //RECT_H