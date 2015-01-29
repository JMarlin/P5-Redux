#ifndef SYSCALL_H
#define SYSCALL_H


extern unsigned int syscall_number;
extern unsigned int syscall_param1;
extern unsigned int syscall_param2;


extern void syscall_handler(void);
void syscall_exec(void);

#endif //SYSCALL_H
