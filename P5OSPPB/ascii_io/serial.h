#ifndef SERIAL_H
#define SERIAL_H

#define SERIAL_DEFAULT_BASE 0x2f8

void initSerial(unsigned short port_base);
void serPutch(unsigned char c);
int serReceived();
char serGetch();


#endif //SERIAL_H
