#ifndef P5_H
#define P5_H

#define KEY_RECIEVED 1
#define PROC_STARTED 2

typedef struct message {

    unsigned int source;
    unsigned int command;
    unsigned int payload;
} message;

extern unsigned int dest
extern unsigned int command
extern unsigned int payload
extern unsigned int retval

extern void asm_get_msg(void);
extern void asm_send_msg(void);

int getMessage(message* msg);
void postMessage(unsigned int dest, unsigned int command, unsigned int payload);
void pchar(char c);
void terminate(void);
unsigned char getch();
void clearScreen();
unsigned int startProc(unsigned char* path);
void scans(int c, char* b);
void prints(char* s);
unsigned char digitToHex(unsigned char digit);
void printHexByte(unsigned char byte);
void printHexWord(unsigned short wd);
void printHexDword(unsigned int dword);

#endif //P5_H
