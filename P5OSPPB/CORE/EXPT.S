.extern _switchToKernel
.globl _except_num

_except_num:
    .byte 0x00, 0x00

.globl _expt_zeroDivide
_expt_zeroDivide: 
    cli
    push %eax 
    mov $0, %al
    mov %al, _except_num
    pop %eax 
    call _switchToKernel


.globl _expt_debugCall
_expt_debugCall: 
    cli
    push %eax 
    mov $1, %al
    mov %al, _except_num
    pop %eax 
    call _switchToKernel


.globl _expt_NMI
_expt_NMI: 
    cli
    push %eax 
    mov $2, %al
    mov %al, _except_num
    pop %eax 
    call _switchToKernel


.globl _expt_breakpoint
_expt_breakpoint: 
    cli
    push %eax 
    mov $3, %al
    mov %al, _except_num
    pop %eax 
    call _switchToKernel


.globl _expt_overflow
_expt_overflow: 
    cli
    push %eax 
    mov $4, %al
    mov %al, _except_num
    pop %eax 
    call _switchToKernel


.globl _expt_outOfBound
_expt_outOfBound: 
    cli
    push %eax 
    mov $5, %al
    mov %al, _except_num
    pop %eax 
    call _switchToKernel


.globl _expt_illegalOpcode
_expt_illegalOpcode: 
    cli
    push %eax 
    mov $6, %al
    mov %al, _except_num
    pop %eax 
    call _switchToKernel


.globl _expt_noCoprocessor
_expt_noCoprocessor: 
    cli
    push %eax 
    mov $7, %al
    mov %al, _except_num
    pop %eax 
    call _switchToKernel


.globl _expt_doubleFault
_expt_doubleFault: 
    cli
    push %eax 
    mov $8, %al
    mov %al, _except_num
    pop %eax 
    call _switchToKernel


.globl _expt_invalidTSS
_expt_invalidTSS: 
    cli
    push %eax 
    mov $10, %al
    mov %al, _except_num
    pop %eax 
    call _switchToKernel


.globl _expt_segNotPresent
_expt_segNotPresent:
    cli
    push %eax 
    mov $11, %al
    mov %al, _except_num
    pop %eax 
    call _switchToKernel


.globl _expt_stackFault
_expt_stackFault: 
    cli
    push %eax 
    mov $12, %al
    mov %al, _except_num
    pop %eax 
    call _switchToKernel


.globl _expt_generalProtection
_expt_generalProtection: 
    cli
    push %eax 
    mov $13, %al
    mov %al, _except_num
    pop %eax 
    call _switchToKernel


.globl _expt_pageFault
_expt_pageFault: 
    cli
    push %eax 
    mov $14, %al
    mov %al, _except_num
    pop %eax 
    call _switchToKernel


.globl _expt_mathFault
_expt_mathFault: 
    cli
    push %eax 
    mov $16, %al
    mov %al, _except_num
    pop %eax 
    call _switchToKernel


.globl _expt_alignCheck
_expt_alignCheck: 
    cli
    push %eax 
    mov $17, %al
    mov %al, _except_num
    pop %eax 
    call _switchToKernel


.globl _expt_machineCheck
_expt_machineCheck: 
    cli
    push %eax 
    mov $18, %al
    mov %al, _except_num
    pop %eax 
    call _switchToKernel


.globl _expt_simdFailure
_expt_simdFailure: 
    cli
    push %eax 
    mov $19, %al
    mov %al, _except_num
    pop %eax 
    call _switchToKernel


.globl _expt_syscall
_expt_syscall:
    cli
    push %eax 
    mov $0xFF, %al
    mov %al, _except_num
    pop %eax 
    call _switchToKernel
