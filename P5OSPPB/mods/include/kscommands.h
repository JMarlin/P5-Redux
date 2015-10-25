#ifndef KSCOMMANDS_H
#define KSCOMMANDS_H

#define KEY_RECIEVED 1
#define PROC_STARTED 2
#define PARAM_DONTCARE 0xFFFFFFFF

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
#define KS_GET_PROC_CPU_PCT 24
#define KS_GET_PID 25
#define KS_PID_FROM_SLOT 26
#define KS_GET_PHYS_PAGE 27
#define KS_FREE_PHYS_PAGE 28
#define KS_GET_SHARED_PAGES 29
#define KS_TIMER 30
#define KS_GET_IMAGE_SIZE 31
#define KS_APPEND_PAGE 32
#define KS_GET_SHARED_PAGE 33

#endif //KSCOMMANDS_H
