#ifndef MESSAGE_H
#define MESSAGE_H

struct process;
struct message;

void passMessage(unsigned int source, unsigned int dest, unsigned int command, unsigned int payload);
int getMessage(struct process* proc, struct message* msgBuf, unsigned int pid_from, unsigned int command);

#endif //MESSAGE_H
