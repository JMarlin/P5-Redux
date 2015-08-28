#ifndef P5_H
#define P5_H

#include "kscommands.h"

typedef struct message {

    unsigned int source;
    unsigned int command;
    unsigned int payload;
    struct message* next;
} message;

extern unsigned int _dest;
extern unsigned int _command;
extern unsigned int _payload;
extern unsigned int _retval;

extern void _asm_get_msg(void);
extern void _asm_send_msg(void);
extern void _asm_get_msg_from(void);

int getMessage(message* msg);
int getMessageFrom(message* msg, unsigned int source, unsigned int command);
void postMessage(unsigned int dest, unsigned int command, unsigned int payload);
void pchar(char c);
void terminate(void);
unsigned char getch();
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

#endif //P5_H
