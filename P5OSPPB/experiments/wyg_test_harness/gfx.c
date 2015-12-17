#include "../../mods/include/p5.h"
#include "../../mods/include/gfx.h"
#include <stdlib.h>
#include <stdio.h>
#include <SDL.h>
#include "../../mods/vesa/font.h" 

screen_mode mode_details;
SDL_Renderer* renderer = (SDL_Renderer*)0;
SDL_Window* window = (SDL_Window*)0;
unsigned int pen_x = 0;
unsigned int pen_y = 0;

unsigned char initGfx() {    
    
    if(SDL_Init(SDL_INIT_VIDEO) < 0) {

        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 0;
    }

    window = SDL_CreateWindow("LESTER", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_SHOWN);

    if(window == NULL) {

        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return 0;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);

    if(renderer == NULL) {

        printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        return 0;
    }
            
    return 1;
}

void endGfx() {
    
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

//Simply returns the number of modes supported
unsigned char enumerateModes() {

    return (unsigned char)(1);
}

screen_mode* getModeDetails(unsigned short modenum) {

    if(!modenum)
        return (screen_mode*)0;

    //Unpack the passed values
    mode_details.width = 800;
    mode_details.height = 600;

    //The bit of math in here is representative of the fact that we support 8-bit
    //16-bit, 24-bit or 32-bit color
    mode_details.depth = 32;
	mode_details.is_linear = 1;
    mode_details.number = 0; //Only the server cares about this

    return &mode_details;
}

unsigned char setScreenMode(unsigned short modenum) {
    
    //Later maybe we'll try to set the window size dynamically later
    return 1;
}

void setColor(unsigned int color) {

    SDL_SetRenderDrawColor(renderer, RVAL(color), GVAL(color), BVAL(color), 0xFF);
}

void setCursor(unsigned short x, unsigned short y) {

    pen_x = x;
    pen_y = y;
}

void setPixel() {

    SDL_RenderDrawPoint(renderer, pen_x, pen_y);
    SDL_RenderPresent(renderer);
}

void drawHLine(unsigned short length) {

    SDL_RenderDrawLine(renderer, pen_x, pen_y, pen_x + length - 1, pen_y);
    SDL_RenderPresent(renderer);
}

void drawVLine(unsigned short length) {

    SDL_RenderDrawLine(renderer, pen_x, pen_y, pen_x, pen_y + length - 1);
    SDL_RenderPresent(renderer);
}

void drawRect(unsigned short width, unsigned short height) {

    SDL_Rect static_rect;

    static_rect.x = pen_x;
    static_rect.y = pen_y;
    static_rect.w = width;
    static_rect.h = height;
    SDL_RenderDrawRect(renderer, &static_rect);
    SDL_RenderPresent(renderer);
}

void fillRect(unsigned short width, unsigned short height) {

    SDL_Rect static_rect;

    static_rect.x = pen_x;
    static_rect.y = pen_y;
    static_rect.w = width;
    static_rect.h = height;
    SDL_RenderFillRect(renderer, &static_rect);
    SDL_RenderPresent(renderer);
}

void drawChar(char c) {

    int j, i;
    unsigned char line;
    c = c & 0x7F; //Reduce to base ASCII set

    for(i = 0; i < 12; i++) {

        line = font_array[i * 128 + c];
        for(j = 0; j < 8; j++) {

            if(line & 0x80) SDL_RenderDrawPoint(renderer, pen_x + j, pen_y + i);
            line = line << 1;
        }
    }
    
    SDL_RenderPresent(renderer);
}

void drawStr(char* str) {

    //Not implemented in production code, so not implemented here
}

bitmap* newBitmap(unsigned int width, unsigned int height) {
    
    unsigned int bmp_size = width * height;
    unsigned int bufsz = (bmp_size *  sizeof(unsigned int)) + sizeof(bitmap);
    bitmap* return_bmp;
    unsigned int i;
        
    if(!(return_bmp = (bitmap*)malloc(bufsz)))
        return (bitmap*)0;
    
    //Set dimensions    
    return_bmp->height = height;
    return_bmp->width = width;
    
    //Default the window to max
    return_bmp->top = 0;
    return_bmp->left = 0;
    return_bmp->bottom = return_bmp->height;
    return_bmp->right = return_bmp->width;
    
    //Plug in the data region
    return_bmp->data = (unsigned int*)((unsigned char*)return_bmp + sizeof(bitmap));
    
    //Clear the bitmap
    for(i = 0; i < bmp_size; i++) {
            
        return_bmp->data[i] = 0;
    }
        
    return return_bmp;
}

void freeBitmap(bitmap* bmp) {
    
    free((void*)bmp);
}

void drawBitmap(bitmap* bmp) {
    
    SDL_Rect srcrect, destrect;
    
    srcrect.x = bmp->left;
    srcrect.y = bmp->top;
    srcrect.w = bmp->right - bmp->left + 1;
    srcrect.h = bmp->bottom - bmp->top + 1;
    
    destrect.x = srcrect.x + pen_x;
    destrect.y = srcrect.y + pen_y;
    destrect.w = srcrect.w;
    destrect.h = srcrect.h; 
    
    printf("srcrect: %u, %u, %u x %u\n", srcrect.x, srcrect.y, srcrect.w, srcrect.h);
    printf("destrect: %u, %u, %u x %u\n", destrect.x, destrect.y, destrect.w, destrect.h);
    
    printf("Creating texture\n");
    SDL_Texture* static_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, bmp->width, bmp->height);
    
    printf("Inserting pixel data into texture\n");
    if(!!SDL_UpdateTexture(static_texture, 0, (void*)bmp->data, 4*bmp->width))
        printf("Couldn't fill texture: %s\n", SDL_GetError());
     
    printf("Copying texture to renderer\n");
    if(!!SDL_RenderCopy(renderer, static_texture, &srcrect, &destrect)) 
        printf("Couldn't render texture: %s\n", SDL_GetError());
    SDL_RenderPresent(renderer);
    
    printf("Cleaning up texture\n");
    SDL_DestroyTexture(static_texture);
}

void copyScreen(bitmap* bmp) {
    
    SDL_Rect pixrect;
    unsigned int color;
    
    pixrect.x = pen_x;
    pixrect.y = pen_y;
    pixrect.w = bmp->width;
    pixrect.h = bmp->height;
    
    SDL_RenderReadPixels(renderer, &pixrect, SDL_PIXELFORMAT_ARGB8888, (void*)(bmp->data), 4*pixrect.w);
}
