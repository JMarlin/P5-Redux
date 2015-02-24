#ifndef EXPT_H
#define EXPT_H

#define EX_ZERODIVIDE      0
#define EX_DEBUGCALL       1
#define EX_NMI             2
#define EX_BREAKPOINT      3
#define EX_OVERFLOW        4
#define EX_OUTOFBOUND      5
#define EX_ILLEGALOPCODE   6
#define EX_NOCOPROCESSOR   7
#define EX_DOUBLEFAULT     8
#define EX_INVALIDTSS      0xA
#define EX_SEGNOTPRESENT   0xB
#define EX_STACKFAULT      0xC
#define EX_GPF             0xD
#define EX_PAGEFAULT       0xE
#define EX_MATHFAULT       0xF
#define EX_ALIGNCHECK      0x10
#define EX_MACHINECHECK    0x11
#define EX_SIMDFAILURE     0x12
#define EX_SYSCALL         0xFF


extern void expt_zeroDivide(void);
extern void expt_debugCall(void);
extern void expt_NMI(void);
extern void expt_breakpoint(void);
extern void expt_overflow(void);
extern void expt_outOfBound(void);
extern void expt_illegalOpcode(void);
extern void expt_noCoprocessor(void);
extern void expt_doubleFault(void);
extern void expt_invalidTSS(void);
extern void expt_segNotPresent(void);
extern void expt_stackFault(void);
extern void expt_generalProtection(void);
extern void expt_pageFault(void);    
extern void expt_mathFault(void);
extern void expt_alignCheck(void);
extern void expt_machineCheck(void);
extern void expt_simdFailure(void);
extern void expt_syscall(void);

extern unsigned char except_num;

#endif //EXPT_H
