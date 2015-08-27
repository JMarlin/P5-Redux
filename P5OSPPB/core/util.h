#ifndef UTIL_H
#define UTIL_H


int enableA20();
int testA20();
void outb(unsigned short _port, unsigned char _data);
unsigned char inb(unsigned short _port);
void outw(unsigned short _port, unsigned short _data);
unsigned short inw(unsigned short _port);
void outd(unsigned short _port, unsigned int _data);
unsigned int ind(unsigned short _port);


#endif //UTIL_H
