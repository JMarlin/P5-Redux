.extern _switchToKernel
.extern _in_kernel
.globl _except_num

_except_num:
    .byte 0x00, 0x00

.globl _expt_zeroDivide
_expt_zeroDivide: 
    incb %ss:_in_kernel
    push %eax 
    mov $0, %al
    mov %al, %ss:_except_num
    pop %eax 
    call _switchToKernel


.globl _expt_debugCall
_expt_debugCall:    
    incb %ss:_in_kernel
    push %eax 
    mov $1, %al
    mov %al, %ss:_except_num
    pop %eax 
    call _switchToKernel


.globl _expt_NMI
_expt_NMI: 
    incb %ss:_in_kernel
    push %eax 
    mov $2, %al
    mov %al, %ss:_except_num
    pop %eax 
    call _switchToKernel


.globl _expt_breakpoint
_expt_breakpoint: 
    incb %ss:_in_kernel
    push %eax 
    mov $3, %al
    mov %al, %ss:_except_num
    pop %eax 
    call _switchToKernel


.globl _expt_overflow
_expt_overflow: 
    incb %ss:_in_kernel
    push %eax 
    mov $4, %al
    mov %al, %ss:_except_num
    pop %eax 
    call _switchToKernel


.globl _expt_outOfBound
_expt_outOfBound: 
    incb %ss:_in_kernel
    push %eax 
    mov $5, %al
    mov %al, %ss:_except_num
    pop %eax 
    call _switchToKernel


.globl _expt_illegalOpcode
_expt_illegalOpcode: 
    incb %ss:_in_kernel
    push %eax 
    mov $6, %al
    mov %al, %ss:_except_num
    pop %eax 
    call _switchToKernel


.globl _expt_noCoprocessor
_expt_noCoprocessor: 
    incb %ss:_in_kernel
    push %eax 
    mov $7, %al
    mov %al, %ss:_except_num
    pop %eax 
    call _switchToKernel


.globl _expt_doubleFault
_expt_doubleFault: 
    incb %ss:_in_kernel
    push %eax 
    mov $8, %al
    mov %al, %ss:_except_num
    pop %eax 
    call _switchToKernel


.globl _expt_invalidTSS
_expt_invalidTSS: 
    incb %ss:_in_kernel
    push %eax 
    mov $10, %al
    mov %al, %ss:_except_num
    pop %eax 
    call _switchToKernel


.globl _expt_segNotPresent
_expt_segNotPresent:
    incb %ss:_in_kernel
    push %eax 
    mov $11, %al
    mov %al, %ss:_except_num
    pop %eax 
    call _switchToKernel


.globl _expt_stackFault
_expt_stackFault: 
    incb %ss:_in_kernel
    push %eax 
    mov $12, %al
    mov %al, %ss:_except_num
    pop %eax 
    call _switchToKernel


.globl _expt_generalProtection
_expt_generalProtection: 
    incb %ss:_in_kernel
    push %eax 
    mov $13, %al
    mov %al, %ss:_except_num
    pop %eax 
    call _switchToKernel


.globl _expt_pageFault
_expt_pageFault: 
    incb %ss:_in_kernel
    push %eax 
    mov $14, %al
    mov %al, %ss:_except_num
    pop %eax 
    call _switchToKernel


.globl _expt_mathFault
_expt_mathFault: 
    incb %ss:_in_kernel
    push %eax 
    mov $16, %al
    mov %al, %ss:_except_num
    pop %eax 
    call _switchToKernel


.globl _expt_alignCheck
_expt_alignCheck: 
    incb %ss:_in_kernel
    push %eax 
    mov $17, %al
    mov %al, %ss:_except_num
    pop %eax 
    call _switchToKernel


.globl _expt_machineCheck
_expt_machineCheck: 
    incb %ss:_in_kernel
    push %eax 
    mov $18, %al
    mov %al, %ss:_except_num
    pop %eax 
    call _switchToKernel


.globl _expt_simdFailure
_expt_simdFailure: 
    incb %ss:_in_kernel
    push %eax 
    mov $19, %al
    mov %al, %ss:_except_num
    pop %eax 
    call _switchToKernel


.globl _expt_syscall
_expt_syscall:
    incb %ss:_in_kernel
    push %eax 
    mov $0xFF, %al
    mov %al, %ss:_except_num
    pop %eax 
    call _switchToKernel

    
.globl _expt_forceenter
_expt_forceenter:
    incb %ss:_in_kernel
    push %eax 
    mov $0xFE, %al
    mov %al, %ss:_except_num
    pop %eax 
    call _switchToKernel    
    