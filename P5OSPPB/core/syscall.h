#ifndef SYSCALL_H
#define SYSCALL_H


extern unsigned int _syscall_number;
extern unsigned int _syscall_param1;
extern unsigned int _syscall_param2;

void set_call_zero_cb(void (*cb)(unsigned int, unsigned int, unsigned int));
extern void syscall_handler(void);
void syscall_exec(void);

#endif //SYSCALL_H
