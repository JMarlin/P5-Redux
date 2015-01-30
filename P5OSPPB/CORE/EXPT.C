#include "expt.h"
#include "../ascii_io/ascii_o.h"
#include "../process/process.h"


void expt_zeroDivide(void) {

    prints("EXCEPTION: DIVIDE BY ZERO!\n");
    terminateProcess((proc*)0x0);    
}


void expt_debugCall(void) {

    prints("EXCEPTION: DEBUGGER CALLED!\n");
    terminateProcess((proc*)0x0);
}


void expt_NMI(void) {

    prints("EXCEPTION: NMI TRIGGERED!\n");
    terminateProcess((proc*)0x0);
}


void expt_breakpoint(void) {

    prints("EXCEPTION: DEBUGGER BREAKPOINT!\n");
    terminateProcess((proc*)0x0);
}


void expt_overflow(void) {

    prints("EXCEPTION: OVERFLOW DETECTED!\n");
    terminateProcess((proc*)0x0);
}


void expt_outOfBound(void) {

    prints("EXCEPTION: OUT OF BOUNDS!\n");
    terminateProcess((proc*)0x0);
}


void expt_illegalOpcode(void) {

    prints("EXCEPTION: ILLEGEAL OPCODE!\n");
    terminateProcess((proc*)0x0);
}


void expt_noCoprocessor(void) {

    prints("EXCEPTION: NO COPROCESSOR!\n");
    terminateProcess((proc*)0x0);
}


void expt_doubleFault(void) {

    prints("EXCEPTION: DOUBLE FAULT!\n");
    terminateProcess((proc*)0x0);
}


void expt_invalidTSS(void) {

    prints("EXCEPTION: INVALID TSS!\n");
    terminateProcess((proc*)0x0);
}


void expt_segNotPresent(void) {

    prints("EXCEPTION: SEGMENT NOT PRESENT!\n");
    terminateProcess((proc*)0x0);
}


void expt_stackFault(void) {

    prints("EXCEPTION: STACK FAULT!\n");
    terminateProcess((proc*)0x0);
}


void expt_generalProtection(void) {

    int i;

    prints("EXCEPTION: PROTECTION FAULT!\n");
    prints("EAX: ");
    asm("\t movl %%eax, %0" : "=r"(i));
    printHexDword(i);
    prints("\n");
    terminateProcess((proc*)0x0);
}


void expt_pageFault(void) {

    prints("EXCEPTION: PAGE FAULT!\n");
    terminateProcess((proc*)0x0);
}


void expt_mathFault(void) {

    prints("EXCEPTION: MATH FAULT!\n");
    terminateProcess((proc*)0x0);
}


void expt_alignCheck(void) {

    prints("EXCEPTION: ALIGNMENT CHECK!\n");
    terminateProcess((proc*)0x0);
}


void expt_machineCheck(void) {

    prints("EXCEPTION: MACHINE CHECK!\n");
    terminateProcess((proc*)0x0);
}


void expt_simdFailure(void) {

    prints("EXCEPTION: SIMD FAILURE!\n");
    terminateProcess((proc*)0x0);
}
