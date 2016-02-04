#include "../include/memory.c"

//Used to delete the elements of a list when those elements are 
void rect_deleter(void* item) {
	
	free(item);
}