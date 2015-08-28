#include "../include/p5.h"
#include "../include/registrar.h"

void main(void) {
	
	message temp_msg;
	unsigned int parent_pid;
	
	//Get the 'here's my pid' message from init
    getMessage(&temp_msg);
    parent_pid = temp_msg.source;	
	prints("[key] Registering keyboard IRQ...");
	
	//Try to register the IRQ
	if(!registerIRQ(1)) {
		
		prints("Failed.");
		postMessage(parent_pid, 0, 0); //Tell the parent we're done registering
    	terminate();
	}
	
	prints("Done.");
	postMessage(parent_pid, 0, 1); //Tell the parent we're done registering
	
	//Now that everything is set up, we can loop waiting for interrupts
	while(1) {
		
		waitForIRQ(1);
		prints("\n[KEY PRESSED]\n");
	}
}