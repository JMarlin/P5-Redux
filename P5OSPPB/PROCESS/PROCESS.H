#ifndef PROCESS_H
#define PROCESS_H

extern int old_esp;
extern int old_cr3;
extern int old_eip;
extern int old_eflags;
extern int old_eax;
extern int old_ecx;
extern int old_edx;
extern int old_ebx;
extern int old_esp;
extern int old_ebp;
extern int old_esi;
extern int old_edi;
extern int old_es;
extern int old_cs;
extern int old_ss;
extern int old_ds;
extern int old_fs;
extern int old_gs;

typedef struct proc {
    int bs;
} proc;

void terminateProcess(proc* p);
void kernelEntry(void);

extern void switchToKernel(void);


#endif //PROCESS_H
