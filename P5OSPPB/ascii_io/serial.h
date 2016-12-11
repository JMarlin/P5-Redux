#ifndef SERIAL_H
#define SERIAL_H


void initSerial();
void serPutch(unsigned char c);
int serReceived();
char serGetch();


#endif //SERIAL_H
