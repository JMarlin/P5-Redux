#ifndef STYLEUTILS_H
#define STYLEUTILS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "context.h"

void draw_panel(Context* context, int x, int y, int width, int height,
               uint32_t color, int border_width, int invert);

#ifdef __cplusplus
}
#endif

#endif //STYLEUTILS_H
