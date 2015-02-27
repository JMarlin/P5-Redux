#ifndef MESSAGE_H
#define MESSAGE_H

#include "process.h"

typedef struct message {

    unsigned int source;
    unsigned int command;
    unsigned int payload;
    struct message* next;
} message;

void passMessage(unsigned int source, unsigned int dest, unsigned int command, unsigned int payload); 
int getMessage(process* proc, message* msgBuf);

#endif //MESSAGE_H
