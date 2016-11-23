
#include "styleutils.h"

void draw_panel(Context* context, int x, int y, int width, int height,
               uint32_t color, int border_width, int invert) {

    uint8_t r = RVAL(color);
    uint8_t g = GVAL(color);
    uint8_t b = BVAL(color);
    uint32_t light_color = RGB(r > 155 ? 255 : r + 100, g > 155 ? 255 : g + 100, b > 155 ? 255 : b + 100);
    uint32_t shade_color = RGB(r < 100 ? 0 : r - 100, g < 100 ? 0 : g - 100, b < 100 ? 0 : b - 100);
    uint32_t temp;
    int i;

    if(invert) {

        temp = shade_color;
        shade_color = light_color;
        light_color = temp;
    }

    for(i = 0; i < border_width; i++) {

        //Top edge
        Context_horizontal_line(context, x+i, y+i, width-(2*i), light_color);

        //Left edge
        Context_vertical_line(context, x+i, y+i+1, height-((i+1)*2), light_color);

        //Bottom edge
        Context_horizontal_line(context, x+i, (y+height)-(i+1), width-(2*i), shade_color);

        //Right edge
        Context_vertical_line(context, x+width-i-1, y+i+1, height-((i+1)*2), shade_color);
    }
}