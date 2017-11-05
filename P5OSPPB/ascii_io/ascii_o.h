#ifndef ASCII_O_H
#define ASCII_O_H

//Comment out to disable the textmode kernel console
#define KPRINTS_ON

void ramdump(unsigned int address, unsigned int count);
unsigned char digitToHex(unsigned char digit);
void printHexByte(unsigned char byte);
void printHexWord(unsigned short wd);
void printHexDword(unsigned int dword);
void klog(char* _str);
void klogHexByte(unsigned char byte);
void klogHexWord(unsigned short wd);
void klogHexDword(unsigned int dword);
void pchar(char _inin);
void prints(char* _str);
void initScreen(int enable_serial);
void setCursor(int x, int y);
void setColor(char newCode);
void clear(void);

#ifdef KPRINTS_ON

void kpchar(unsigned char c);
void kprints(unsigned char* s);
void kprintHexByte(unsigned char byte);
void kprintHexWord(unsigned short wd);
void kprintHexDword(unsigned int dword);
void enterTextMode(void (*cb)(void));

#define KPCHAR(x) kpchar(x)
#define KPRINTS(x) kprints(x)
#define KPRINTHEXBYTE(x) kprintHexByte(x)
#define KPRINTHEXWORD(x) kprintHexWord(x)
#define KPRINTHEXDWORD(x) kprintHexDword(x)

#else

#define KPCHAR(x)
#define KPRINTS(x)
#define KPRINTHEXBYTE(x)
#define KPRINTHEXWORD(x)
#define KPRINTHEXDWORD(x)

#endif //KPRINTS_ON

#endif //ASCII_O_H
