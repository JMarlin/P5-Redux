#ifndef P5_H
#define P5_H

#include "kscommands.h"

#define BRK() ((int*)0x0)[0] = 0
#define MSG_STRLEN 34
#define MSG_STRCHUNK 35 

typedef struct message {

    unsigned int source;
    unsigned int command;
    unsigned int payload;
} message;

extern unsigned int _dest;
extern unsigned int _command;
extern unsigned int _payload;
extern unsigned int _retval;

extern void _asm_get_msg(void);
extern void _asm_send_msg(void);
extern void _asm_get_msg_from(void);
extern void _asm_message_exists(void);

int getMessage(message* msg);
int messageExists(unsigned int source, unsigned int command);
int getMessageFrom(message* msg, unsigned int source, unsigned int command);
void postMessage(unsigned int dest, unsigned int command, unsigned int payload);
unsigned int getCurrentPid(void);
unsigned int getProcessCPUUsage(unsigned int pid);
unsigned int registerIRQ(unsigned int irq_number);
void waitForIRQ(unsigned int irq_number);
void pchar(char c);
void terminate(void);
void clearScreen();
unsigned int startProc(unsigned char* path);
unsigned int startSuperProc(unsigned char* path);
unsigned int startV86(unsigned char* path);
void scans(int c, char* b);
void prints(char* s);
unsigned char digitToHex(unsigned char digit);
void printHexByte(unsigned char byte);
void printHexWord(unsigned short wd);
void printHexDword(unsigned int dword);
unsigned int getBuildNumber(void);
unsigned int getNextPid();
void resetPidSearch();
void* allocatePhysical(void* base_address, unsigned int byte_count);
unsigned char freePhysical(void* base_address, unsigned int byte_count);
void* getSharedPages(unsigned int count);
void freeSharedPages(void* base);
void* getSharedPage(void);
unsigned int getImageSize(unsigned int pid);
unsigned int appendPage(void);
void printDecimal(unsigned int dword);
void sendString(unsigned char* s, unsigned int dest);
unsigned int getStringLength(unsigned int src);
void getString(unsigned int src, unsigned char* outstring, unsigned int count);
void installExceptionHandler(void* handler);
unsigned int getElapsedMs(void);
unsigned int sleep(unsigned int ms);
int startThread(void);

#endif //P5_H
