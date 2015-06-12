#ifndef ASCII_O_H
#define ASCII_O_H

void ramdump(unsigned int address, unsigned int count);
unsigned char digitToHex(unsigned char digit);
void printHexByte(unsigned char byte);
void printHexWord(unsigned short wd);
void printHexDword(unsigned int dword);
void pchar(char _inin);
void prints(char* _str);
void initScreen();
void setCursor(int x, int y);
void setColor(char newCode);
void clear(void);

#endif //ASCII_O_H
