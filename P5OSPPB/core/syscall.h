#ifndef SYSCALL_H
#define SYSCALL_H


extern unsigned int _syscall_number;
extern unsigned int _syscall_param1;
extern unsigned int _syscall_param2;


extern void syscall_handler(void);
void syscall_exec(void);

#endif //SYSCALL_H
