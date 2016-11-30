#ifndef PROCESS_H
#define PROCESS_H

#define PT_USER 0
#define PT_V86  1

//Process flags
#define PF_SUPER 1 //If the flag at bit 1 is set then the process has supervisor permissions
#define PF_V86   2 //Process is 16-bit
#define PF_DEBUG    4 //Process will single-step
#define PF_WAITMSG  8 //Process is halted waiting to recieve a message
#define PF_WOKENMSG 16 //Process just woke up from a message
#define PF_NONSYS 32 //Process was not started by the kernel

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

typedef void (*entry_func)(void);

typedef struct process {
    unsigned int id;
    struct pageRange* root_page;
    struct message* root_msg;
    context ctx;
    void* usr;
    unsigned int base;
    unsigned int size;
    unsigned int flags;
    unsigned int wait_pid;
    unsigned int wait_cmd;
    unsigned int called_count;
    unsigned int cpu_pct;
    unsigned char* name;
	entry_func exception_handler;
	unsigned char in_exception;
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

typedef struct ivector {
    unsigned short offset;
    unsigned short segment;
} __attribute__ ((packed)) ivector;

process* p;
process* procTable;

void startProcessManagement();
void endProc(process* proc);
void deleteProc(process* proc);
void kernelEntry(void);
void resetProcessCounter(int preserve_ids);
process* newUserProc();
process* newSuperProc();
//process* newV86Proc();
void setProcEntry(process* proc, void* entryPoint);
void enterProc(unsigned int pid);
unsigned int exec_loaded_v86(unsigned int app_size);
unsigned int exec_process(unsigned char* path, char isSuper);
unsigned int exec_v86(unsigned char* path);
int request_new_page(process* proc);
void next_process();
void prep_next_process();
void returnToProcess(process* proc);
process* makeThread(process* parent, void* entry_point);
void force_into_exception(process* proc);

extern void _switchToKernel(void);
extern void _returnToProc(void);

#endif //PROCESS_H
