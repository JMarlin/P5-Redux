#include "../include/memory.h"
#include "rect.h"

//Used to delete the elements of a list when those elements are 
void Rect_deleter(void* item) {
	
	free(item);
}

Rect* Rect_new(unsigned int top, unsigned int left, unsigned int bottom, unsigned int right) {
    
    Rect* rect = (Rect*)malloc(sizeof(Rect));
    	
    if(!rect)
        return rect;
    
    rect->top = top;
    rect->left = left;
    rect->bottom = bottom;
    rect->right = right;

    return rect;
}
