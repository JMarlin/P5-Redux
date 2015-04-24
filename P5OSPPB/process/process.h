#ifndef PROCESS_H
#define PROCESS_H

#define PT_USER 0
#define PT_V86  1

//Process flags
#define PF_SUPER 1 //If the flag at bit 1 is set then the process has supervisor permissions
#define PF_V86   2
#define PF_DEBUG    4 //Process will single-step

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

struct message;
struct pageRange;

typedef struct process {
    unsigned int id;
    struct pageRange* root_page;
    struct message* root_msg;
    context ctx;
    void* usr;
    unsigned int base;
    unsigned int size;
    unsigned int flags;
} process;

extern int _old_esp;
extern int _old_cr3;
extern int _old_eip;
extern int _old_eflags;
extern int _old_eax;
extern int _old_ecx;
extern int _old_edx;
extern int _old_ebx;
extern int _old_ebp;
extern int _old_esi;
extern int _old_edi;
extern short _old_es;
extern short _old_cs;
extern short _old_ss;
extern short _old_ds;
extern short _old_fs;
extern short _old_gs;
extern int _old_err;

extern unsigned char _in_kernel;
extern unsigned char _prc_is_super;

unsigned char needs_swap;

typedef struct ivector {
    unsigned short offset;
    unsigned short segment;
} __attribute__ ((packed)) ivector;

process* p;
process* procTable;

void startProcessManagement();
void endProc(process* proc);
void kernelEntry(void);
process* newUserProc();
process* newSuperProc();
//process* newV86Proc();
void setProcEntry(process* proc, void* entryPoint);
void enterProc(unsigned int pid);
unsigned int exec_process(unsigned char* path, char isSuper);
int request_new_page(process* proc);
void next_process();
void prep_next_process();

extern void _switchToKernel(void);
extern void _returnToProc(void);

#endif //PROCESS_H