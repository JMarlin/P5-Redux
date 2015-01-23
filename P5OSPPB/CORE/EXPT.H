#ifndef EXPT_C
#define EXPT_C

void expt_zeroDivide(void);
void expt_debugCall(void);
void expt_NMI(void);
void expt_breakpoint(void);
void expt_overflow(void);
void expt_outOfBound(void);
void expt_illegalOpcode(void);
void expt_noCoprocessor(void);
void expt_doubleFault(void);
void expt_invalidTSS(void);
void expt_segNotPresent(void);
void expt_stackFault(void);
void expt_generalProtection(void);
void expt_pageFault(void);    
void expt_mathFault(void);
void expt_alignCheck(void);
void expt_machineCheck(void);
void expt_simdFailure(void);


#endif #EXPT_C
