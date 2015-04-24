#ifndef UTIL_H
#define UTIL_H


int enableA20();
int testA20();
inline void outb(unsigned short _port, unsigned char _data);
inline unsigned char inb(unsigned short _port);
inline void outw(unsigned short _port, unsigned short _data);
inline unsigned short inw(unsigned short _port);
inline void outd(unsigned short _port, unsigned int _data);
inline unsigned int ind(unsigned short _port);


#endif //UTIL_H