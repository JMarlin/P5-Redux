#include "../include/p5.h"


void main(void) {
    
    message temp_msg;
    unsigned int temp_payload = 0;    
        
    while(1) {
        while(!getMessage(&temp_msg));
      
        postMessage(temp_msg.source, 1, temp_payload++);
    }
    
}
