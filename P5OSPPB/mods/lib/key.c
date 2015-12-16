#include "../include/p5.h"
#include "../include/registrar.h"
#include "../include/key.h"

unsigned int key_pid;
message temp_msg;

int initKey() {
	
	//Find the KEY server
	postMessage(REGISTRAR_PID, REG_LOOKUP, SVC_KEY);
    getMessage(&temp_msg);
    key_pid = temp_msg.payload;

    return key_pid != 0;
}

unsigned char getch() {
	
	prints("Getting key...");
	
	postMessage(key_pid, KEY_GETCH, 0);
	prints("message posted...");
	getMessageFrom(&temp_msg, key_pid, KEY_GETCH);
	
	prints("got key\n");
	
	return (unsigned char)(temp_msg.payload & 0xFF);
}
