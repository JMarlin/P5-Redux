#ifndef PROCESS_H
#define PROCESS_H

#include "../memory/paging.h"

#define PT_USER 0
#define PT_V86  1


extern int old_esp;
extern int old_cr3;
extern int old_eip;
extern int old_eflags;
extern int old_eax;
extern int old_ecx;
extern int old_edx;
extern int old_ebx;
extern int old_ebp;
extern int old_esi;
extern int old_edi;
extern short old_es;
extern short old_cs;
extern short old_ss;
extern short old_ds;
extern short old_fs;
extern short old_gs;
extern int old_err;

extern unsigned char in_kernel;

typedef struct proc {
    int bs;
} proc;

typedef struct ivector {
    unsigned short offset;
    unsigned short segment;
} __attribute__ ((packed)) ivector;

typedef struct context {
    unsigned int esp;
    unsigned int cr3;
    unsigned int eip;
    unsigned int eflags;
    unsigned int eax;
    unsigned int ecx;
    unsigned int edx;
    unsigned int ebx;
    unsigned int ebp;
    unsigned int esi;
    unsigned int edi;
    unsigned short es;
    unsigned short cs;
    unsigned short ss;
    unsigned short ds;
    unsigned short fs;
    unsigned short gs;
    unsigned int err;
    unsigned char vif;
    unsigned char type;
} context;

typedef struct process {
    unsigned int id;
    pageRange* root_page;
    context ctx;
    void* usr;
    unsigned int base;
    unsigned int size;
} process;

process* p;

void startProcessManagement();
void endProc(process* proc);
void kernelEntry(void);
process* newUserProc();
//process* newV86Proc();
void setProcEntry(process* proc, void* entryPoint);
void startProc(process* proc);
process* exec_process(unsigned char* path);
int request_new_page(process* proc);
void next_process();
void prep_next_process();

extern void switchToKernel(void);
extern void returnToProc(void);

extern unsigned char swapping;

#endif //PROCESS_H
