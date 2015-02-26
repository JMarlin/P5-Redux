#ifndef P5_H
#define P5_H


void pchar(char c);
unsigned char getch();
void prints(char* s);
void terminate(void);
void scans(int c, char* b);
void clearScreen();
void startProc();
void nextProc();
void endProc();
void printHexByte(unsigned char byte);
void printHexWord(unsigned short wd);
void printHexDword(unsigned int dword);
unsigned int getTimeCounter();


#endif //P5_H
