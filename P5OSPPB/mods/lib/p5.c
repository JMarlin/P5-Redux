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

unsigned int current_process = 0;
void resetPidSearch() {

    current_process = 0;
}

unsigned int getNextPid() {

    unsigned int return_pid = 0;

    if(current_process == 256)
        return 0;

    while(!return_pid) {

        postMessage(0, KS_PID_FROM_SLOT, current_process);
        getMessageFrom(&temp_msg, 0, KS_PID_FROM_SLOT);
        return_pid = temp_msg.payload;

        if(current_process == 255) {

            if(return_pid) {

                break;
            } else {

                current_process++;
                return 0;
            }
        } else {

            current_process++;
        }
    }

    return return_pid;
}

unsigned int getCurrentPid() {

    postMessage(0, KS_GET_PID, 0);
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

int startThread() {
    
    postMessage(0, KS_START_THREAD, 0);
    getMessageFrom(&temp_msg, 0, KS_START_THREAD);
    
    return temp_msg.payload;
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

void* allocatePhysical(void* base_address, unsigned int byte_count) {

    unsigned int i;
    unsigned int page_count = (byte_count >> 12) + ((byte_count & 0xFFF) ? 1 : 0);
    unsigned int alloc_address;

    base_address = (void *)((unsigned int)base_address & 0xFFFFF000);
    alloc_address = (unsigned int)base_address;

    for(i = 0; i < page_count; i++, alloc_address += 0x1000) {

        postMessage(0, KS_GET_PHYS_PAGE, alloc_address);
        getMessageFrom(&temp_msg, 0, KS_GET_PHYS_PAGE);

        if(!temp_msg.payload) {

            freePhysical(base_address, byte_count);
            return (void*)0;
        }
    }

    return base_address;
}

unsigned char freePhysical(void* base_address, unsigned int byte_count) {

    unsigned int i;
    unsigned int page_count = (byte_count >> 12) + ((byte_count & 0xFFF) ? 1 : 0);
    unsigned int alloc_address;

    base_address = (void *)((unsigned int)base_address & 0xFFFFF000);
    alloc_address = (unsigned int)base_address;

    for(i = 0; i < page_count; i++, alloc_address += 0x1000) {

        postMessage(0, KS_FREE_PHYS_PAGE, alloc_address);
        getMessageFrom(&temp_msg, 0, KS_FREE_PHYS_PAGE);

        if(!temp_msg.payload)
            return temp_msg.payload;
    }

    return 1;
}

void* getSharedPages(unsigned int count) {

    postMessage(0, KS_GET_SHARED_PAGES, count);
    getMessageFrom(&temp_msg, 0, KS_GET_SHARED_PAGES);

    return (void*)temp_msg.payload;
}

void freeSharedPages(void* base) {
    
    //To be implemented later
}

void* getSharedPage(void) {

    postMessage(0, KS_GET_SHARED_PAGE, 0);
    getMessageFrom(&temp_msg, 0, KS_GET_SHARED_PAGE);

    return (void*)temp_msg.payload;
}

unsigned int sleep(unsigned int ms) {

    postMessage(0, KS_TIMER, ms);
    getMessageFrom(&temp_msg, 0, KS_TIMER);

    return temp_msg.payload;
}

unsigned int getImageSize(unsigned int pid) {

    postMessage(0, KS_GET_IMAGE_SIZE, pid);
    getMessageFrom(&temp_msg, 0, KS_GET_IMAGE_SIZE);

    return temp_msg.payload;
}

unsigned int appendPage(void) {
    
    //prints("[lib] Sending append request\n");
    postMessage(0, KS_APPEND_PAGE, 0);
    //prints("[lib] Waiting for response\n");
    getMessageFrom(&temp_msg, 0, KS_APPEND_PAGE);
    //prints("[lib] Got response\n");
    
    return temp_msg.payload;
}

void printDecimal(unsigned int dword) {

    unsigned char digit[12];
    int i, j;

    i = 0;
    while(1) {

        if(!dword) {

            if(i == 0)
                digit[i++] = 0;

            break;
        }

        digit[i++] = dword % 10;
        dword /= 10;
    }

    for(j = i - 1; j >= 0; j--)
        pchar(digit[j] + '0');
}

void sendString(unsigned char* s, unsigned int dest) {

    unsigned int strlen, i, s_index, transfer_chunk;
    
    //Count the string length 
    for(strlen = 0; s[strlen]; strlen++);
    
    //Set up the countdown value
    s_index = 0;
    
    //Make sure the value reflects the length, not the highest index
    strlen++;
    
    //Tell the destination process that we want to send a string 
    //of ceil(strlen/4) string chunks 
    postMessage(dest, MSG_STRLEN, strlen/4 + (strlen%4 ? 1 : 0));
    getMessageFrom(&temp_msg, dest, MSG_STRLEN);
    
    while(s_index < strlen) {
            
        //Pack a transfer chunk
        transfer_chunk = 0;
            
        for(i = 0; i < 4 && s_index < strlen; i++, s_index++)   
            transfer_chunk |= (((unsigned int)s[s_index]) << (8*i));    
            
        //Send the chunk
        postMessage(dest, MSG_STRCHUNK, transfer_chunk);
        getMessageFrom(&temp_msg, dest, MSG_STRCHUNK);
    }
}

unsigned int getStringLength(unsigned int src) {
    
    //Get the number of chunks from the source 
    getMessageFrom(&temp_msg, src, MSG_STRLEN);
    postMessage(src, MSG_STRLEN, 1);
    return temp_msg.payload * sizeof(unsigned int);
}

void getString(unsigned int src, unsigned char* outstring, unsigned int count) {
    
    unsigned int chunk_count, recieved, s_index, i;
    
    //Convert from characters to chunks
    chunk_count = count/sizeof(unsigned int) + (count%sizeof(unsigned int) ? 1 : 0);
    
    //Start at zero
    recieved = 0;
    s_index = 0;
    
    while(recieved < chunk_count) {
        
        //Get the chunks from the client
        getMessageFrom(&temp_msg, src, MSG_STRCHUNK);
        postMessage(src, MSG_STRCHUNK, 1);
        
        //Unpack the chunk into the string         
        for(i = 0; i < 4; i++)
            outstring[s_index++] = (unsigned char)((temp_msg.payload >> (i*8)) & 0xFF);
        
        recieved++;
    }
    
    //Make sure the string is zero-terminated
    outstring[s_index] = 0;
}

void installExceptionHandler(void* handler) {
	
	postMessage(0, KS_INSTALL_EXHDLR, (unsigned int)handler);
}

unsigned int getElapsedMs() {

    postMessage(0, KS_GET_ELAPSED_MS, 0);
    getMessageFrom(&temp_msg, 0, KS_GET_ELAPSED_MS);

    return temp_msg.payload;
}
