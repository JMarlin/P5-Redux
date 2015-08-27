#ifndef KSERVER_H
#define KSERVER_H

//Commands to send back to requesters
#define KEY_RECIEVED 1
#define PROC_STARTED 2
#define BUILD_NUMBER 3

//Commands which the kernel server understands
#define KS_QUIT 0
#define KS_PCHAR 1
#define KS_GETCH 2
#define KS_CLEAR_SCREEN 3
#define KS_EXEC 4
#define KS_EXEC_SUPER 5
#define KS_EXEC_V86 6
#define KS_TOGGLE_DEBUG  7
#define KS_GET_BUILD_NO  8
#define KS_REG_IRQ_1 9
#define KS_REG_IRQ_2 10
#define KS_REG_IRQ_3 11
#define KS_REG_IRQ_4 12
#define KS_REG_IRQ_5 13
#define KS_REG_IRQ_6 14
#define KS_REG_IRQ_7 15
#define KS_REG_IRQ_8 16
#define KS_REG_IRQ_9 17
#define KS_REG_IRQ_A 18
#define KS_REG_IRQ_B 19
#define KS_REG_IRQ_C 20
#define KS_REG_IRQ_D 21
#define KS_REG_IRQ_E 22
#define KS_REG_IRQ_F 23

void post_to_kern(unsigned int source, unsigned int command, unsigned int payload);

#endif //KSERVER_H
