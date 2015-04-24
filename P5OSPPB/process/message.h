#ifndef MESSAGE_H
#define MESSAGE_H

typedef struct message {

    unsigned int source;
    unsigned int command;
    unsigned int payload;
    struct message* next;
} message;

struct process;

void passMessage(unsigned int source, unsigned int dest, unsigned int command, unsigned int payload); 
int getMessage(struct process* proc, message* msgBuf);

#endif //MESSAGE_H
