#include "../include/p5.h"


message temp_msg;


int getMessage(message* msg) {

    _asm_get_msg();

    if(!_retval)
        return 0;

    msg->source = _dest;
    msg->command = _command;
    msg->payload = _payload;
    return 1;
}


int getMessageFrom(message* msg, unsigned int source, unsigned int command) {

    _dest = source;
    _command = command;

    _asm_get_msg_from();

    if(!_retval)
        return 0;

    msg->source = _dest;
    msg->command = _command;
    msg->payload = _payload;
    return 1;
}


void postMessage(unsigned int ldest, unsigned int lcommand, unsigned int lpayload)  {

    _dest = ldest;
    _command = lcommand;
    _payload = lpayload;
    _asm_send_msg();
}

unsigned int getCurrentPid(void) {
    
    postMessage(0, KS_GET_PID, pid);
    getMessageFrom(&temp_msg, 0, KS_GET_PID);
    
    return temp_msg.payload;
}

unsigned int getProcessCPUUsage(unsigned int pid) {
    
    postMessage(0, KS_GET_PROC_CPU_PCT, pid);
    getMessageFrom(&temp_msg, 0, KS_GET_PROC_CPU_PCT);
    
    return temp_msg.payload;
}

unsigned int registerIRQ(unsigned int irq_number) {
    
    //Post a request to register the IRQ
    postMessage(0, KS_REG_IRQ_1 + (irq_number - 1), 0);
    
    //Wait for the reply from the kernel
    getMessageFrom(&temp_msg, 0, KS_REG_IRQ_1 + irq_number - 1);
    
    //Tell the requester whether or not the registration succeeded
    return temp_msg.payload;
}

//Sleep the process until the kernel passes it an interrupt message
void waitForIRQ(unsigned int irq_number) {
    
    getMessageFrom(&temp_msg, 0, KS_REG_IRQ_1 + (irq_number - 1));
}

void pchar(char c) {

    postMessage(0, 1, (unsigned int)c);
}


void terminate(void) {

    postMessage(0, 0, 0);
}


unsigned char getch() {

    postMessage(0, 2, 0);

    //We should probably add a method to ignore messages
    //we don't care about but leave them in the queue
    while(!getMessageFrom(&temp_msg, 0, KS_GETCH));

    return (unsigned char)(temp_msg.payload & 0xFF);
}


void clearScreen() {

    postMessage(0, 3, 0);
}


unsigned int startProc(unsigned char* path) {

    postMessage(0, 4, (unsigned int)path);
    //We should probably add a method to ignore messages
    //we don't care about but leave them in the queue
    while(!getMessage(&temp_msg));

    return temp_msg.payload;
}


unsigned int startSuperProc(unsigned char* path) {

    postMessage(0, 5, (unsigned int)path);

    //We should probably add a method to ignore messages
    //we don't care about but leave them in the queue
    while(!getMessage(&temp_msg));

    return temp_msg.payload;
}


unsigned int startV86(unsigned char* path) {

    postMessage(0, 6, (unsigned int)path);

    //We should probably add a method to ignore messages
    //we don't care about but leave them in the queue
    while(!getMessage(&temp_msg));

    return temp_msg.payload;
}


void scans(int c, char* b) {

    unsigned char temp_char;
    int index = 0;

    for(index = 0 ; index < c-1 ; ) {
        temp_char = getch();

        if(temp_char != 0) {
            b[index] = temp_char;
            pchar(b[index]);

            if(b[index] == '\n') {
                b[index] = 0;
                break;
            }

            index++;

            if(index == c-1)
                pchar('\n');
        }
    }

    b[index+1] = 0;
}


void prints(char* s) {

    int index = 0;

    while(s[index] != 0) {
        pchar(s[index]);
        index++;
    }
}

unsigned char digitToHex(unsigned char digit) {

    if(digit < 0xA) {
        return (digit + '0');
    } else {
        return ((digit - 0xA) + 'A');
    }
}


void printHexByte(unsigned char byte) {

    pchar(digitToHex((byte & 0xF0)>>4));
    pchar(digitToHex(byte & 0xF));
}


void printHexWord(unsigned short wd) {

    printHexByte((unsigned char)((wd & 0xFF00)>>8));
    printHexByte((unsigned char)(wd & 0xFF));
}


void printHexDword(unsigned int dword) {

    printHexWord((unsigned short)((dword & 0xFFFF0000)>>16));
    printHexWord((unsigned short)(dword & 0xFFFF));
}


unsigned int getBuildNumber(void) {

    postMessage(0, 8, 0);

    //We should probably add a method to ignore messages
    //we don't care about but leave them in the queue
    while(!getMessage(&temp_msg));

    return temp_msg.payload;
}
