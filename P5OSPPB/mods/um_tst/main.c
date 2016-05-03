#include "../include/p5.h"
#include "../include/key.h"

void main() {

	if(!initKey()) {
	    
	    prints("Couldn't access the KEY service!");
	    terminate();
	}

	prints("::");
	
	while(1) {
            
	    pchar(getch());
	}
}

