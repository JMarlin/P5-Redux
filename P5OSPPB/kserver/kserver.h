#ifndef KSERVER_H
#define KSERVER_H

#define KEY_RECIEVED 1
#define PROC_STARTED 2

void post_to_kern(unsigned int source, unsigned int command, unsigned int payload);

#endif //KSERVER_H
