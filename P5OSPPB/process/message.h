#ifndef MESSAGE_H
#define MESSAGE_H

struct process;

typedef struct message {

    unsigned int source;
    unsigned int command;
    unsigned int payload;
    struct message* next;
} message;

void passMessage(unsigned int source, unsigned int dest, unsigned int command, unsigned int payload);
int getMessage(struct process* proc, message* msgBuf, unsigned int pid_from, unsigned int command);

#endif //MESSAGE_H
