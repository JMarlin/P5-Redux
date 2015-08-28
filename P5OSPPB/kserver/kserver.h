#ifndef KSERVER_H
#define KSERVER_H

//Commands to send back to requesters
#define KEY_RECIEVED 1
#define PROC_STARTED 2
#define BUILD_NUMBER 3

void post_to_kern(unsigned int source, unsigned int command, unsigned int payload);

#endif //KSERVER_H
