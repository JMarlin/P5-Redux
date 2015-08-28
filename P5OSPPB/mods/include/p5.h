#ifndef P5_H
#define P5_H

#define KEY_RECIEVED 1
#define PROC_STARTED 2
#define PARAM_DONTCARE 0xFFFFFFFF

//Commands which the kernel server understands
#define KS_QUIT 0
#define KS_PCHAR 1
#define KS_GETCH 2
#define KS_CLEAR_SCREEN 3
#define KS_EXEC 4
#define KS_EXEC_SUPER 5
#define KS_EXEC_V86 6
#define KS_TOGGLE_DEBUG  7
#define KS_GET_BUILD_NO  8
#define KS_REG_IRQ_1 9
#define KS_REG_IRQ_2 10
#define KS_REG_IRQ_3 11
#define KS_REG_IRQ_4 12
#define KS_REG_IRQ_5 13
#define KS_REG_IRQ_6 14
#define KS_REG_IRQ_7 15
#define KS_REG_IRQ_8 16
#define KS_REG_IRQ_9 17
#define KS_REG_IRQ_A 18
#define KS_REG_IRQ_B 19
#define KS_REG_IRQ_C 20
#define KS_REG_IRQ_D 21
#define KS_REG_IRQ_E 22
#define KS_REG_IRQ_F 23

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
