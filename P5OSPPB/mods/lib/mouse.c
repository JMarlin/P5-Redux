#include "../include/p5.h"
#include "../include/mouse.h"
#include "../include/registrar.h"

unsigned int mouse_pid;

int initMouse() {
		
	message temp_msg;	
		
	//Get mouse provider from registrar
    //In the future, we should update registrar to enable the registration and detection
	//of multiples modules all providing the same service class so that if, for instance,
	//both the USB module and the PS2 module detect and register a mouse then we want to 
	//subscribe to messages from both of them. Right now, we just get the first service 
	//that was registered
	postMessage(REGISTRAR_PID, REG_LOOKUP, SVC_MOUSE);
    getMessage(&temp_msg);
    if(!(mouse_pid = temp_msg.payload))
	    return 0;
	
	//Send a 'register me' message to the provider
	postMessage(mouse_pid, MOUSE_REG_LISTENER, 1);
	getMessageFrom(&temp_msg, mouse_pid, MOUSE_REG_LISTENER);
	 
	//After this, the client should be prepared to recieve mouse update messages in its message queue
	return temp_msg.payload;
}