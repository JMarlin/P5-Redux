.extern _switchToKernel
.extern _in_kernel
.globl _except_num
.globl _temp_eax

_except_num:
    .byte 0x00, 0x00
_temp_eax: .byte 0x00

.globl _expt_zeroDivide
_expt_zeroDivide:
    incb $0x0010:_in_kernel
    mov %eax, $0x0010:_temp_eax
    mov $0, %al
    mov %al, $0x0010:_except_num
    mov $0x0010:_temp_eax, %eax
    call _switchToKernel


.globl _expt_debugCall
_expt_debugCall:
    incb $0x0010:_in_kernel
    mov %eax, $0x0010:_temp_eax
    mov $1, %al
    mov %al, $0x0010:_except_num
    mov $0x0010:_temp_eax, %eax
    call _switchToKernel


.globl _expt_NMI
_expt_NMI:
    incb $0x0010:_in_kernel
    mov %eax, $0x0010:_temp_eax
    mov $2, %al
    mov %al, $0x0010:_except_num
    mov $0x0010:_temp_eax, %eax
    call _switchToKernel


.globl _expt_breakpoint
_expt_breakpoint:
    incb $0x0010:_in_kernel
    mov %eax, $0x0010:_temp_eax
    mov $3, %al
    mov %al, $0x0010:_except_num
    mov $0x0010:_temp_eax, %eax
    call _switchToKernel


.globl _expt_overflow
_expt_overflow:
    incb $0x0010:_in_kernel
    mov %eax, $0x0010:_temp_eax
    mov $4, %al
    mov %al, $0x0010:_except_num
    mov $0x0010:_temp_eax, %eax
    call _switchToKernel


.globl _expt_outOfBound
_expt_outOfBound:
    incb $0x0010:_in_kernel
    mov %eax, $0x0010:_temp_eax
    mov $5, %al
    mov %al, $0x0010:_except_num
    mov $0x0010:_temp_eax, %eax
    call _switchToKernel


.globl _expt_illegalOpcode
_expt_illegalOpcode:
    incb $0x0010:_in_kernel
    mov %eax, $0x0010:_temp_eax
    mov $6, %al
    mov %al, $0x0010:_except_num
    mov $0x0010:_temp_eax, %eax
    call _switchToKernel


.globl _expt_noCoprocessor
_expt_noCoprocessor:
    incb $0x0010:_in_kernel
    mov %eax, $0x0010:_temp_eax
    mov $7, %al
    mov %al, $0x0010:_except_num
    mov $0x0010:_temp_eax, %eax
    call _switchToKernel


.globl _expt_doubleFault
_expt_doubleFault:
    incb $0x0010:_in_kernel
    mov %eax, $0x0010:_temp_eax
    mov $8, %al
    mov %al, $0x0010:_except_num
    mov $0x0010:_temp_eax, %eax
    call _switchToKernel


.globl _expt_invalidTSS
_expt_invalidTSS:
    incb $0x0010:_in_kernel
    mov %eax, $0x0010:_temp_eax
    mov $10, %al
    mov %al, $0x0010:_except_num
    mov $0x0010:_temp_eax, %eax
    call _switchToKernel


.globl _expt_segNotPresent
_expt_segNotPresent:
    incb $0x0010:_in_kernel
    mov %eax, $0x0010:_temp_eax
    mov $11, %al
    mov %al, $0x0010:_except_num
    mov $0x0010:_temp_eax, %eax
    call _switchToKernel


.globl _expt_stackFault
_expt_stackFault:
    incb $0x0010:_in_kernel
    mov %eax, $0x0010:_temp_eax
    mov $12, %al
    mov %al, $0x0010:_except_num
    mov $0x0010:_temp_eax, %eax
    call _switchToKernel


.globl _expt_generalProtection
_expt_generalProtection:
    incb $0x0010:_in_kernel
    mov %eax, $0x0010:_temp_eax
    mov $13, %al
    mov %al, $0x0010:_except_num
    mov $0x0010:_temp_eax, %eax
    call _switchToKernel


.globl _expt_pageFault
_expt_pageFault:
    incb $0x0010:_in_kernel
    mov %eax, $0x0010:_temp_eax
    mov $14, %al
    mov %al, $0x0010:_except_num
    mov $0x0010:_temp_eax, %eax
    call _switchToKernel


.globl _expt_mathFault
_expt_mathFault:
    incb $0x0010:_in_kernel
    mov %eax, $0x0010:_temp_eax
    mov $16, %al
    mov %al, $0x0010:_except_num
    mov $0x0010:_temp_eax, %eax
    call _switchToKernel


.globl _expt_alignCheck
_expt_alignCheck:
    incb $0x0010:_in_kernel
    mov %eax, $0x0010:_temp_eax
    mov $17, %al
    mov %al, $0x0010:_except_num
    mov $0x0010:_temp_eax, %eax
    call _switchToKernel


.globl _expt_machineCheck
_expt_machineCheck:
    incb $0x0010:_in_kernel
    mov %eax, $0x0010:_temp_eax
    mov $18, %al
    mov %al, $0x0010:_except_num
    mov $0x0010:_temp_eax, %eax
    call _switchToKernel


.globl _expt_simdFailure
_expt_simdFailure:
    incb $0x0010:_in_kernel
    mov %eax, $0x0010:_temp_eax
    mov $19, %al
    mov %al, $0x0010:_except_num
    mov $0x0010:_temp_eax, %eax
    call _switchToKernel


.globl _expt_syscall
_expt_syscall:
    incb $0x0010:_in_kernel
    mov %eax, $0x0010:_temp_eax
    mov $0xFF, %al
    mov %al, $0x0010:_except_num
    mov $0x0010:_temp_eax, %eax
    call _switchToKernel


.globl _expt_forceenter
_expt_forceenter:
    incb $0x0010:_in_kernel
    mov %eax, $0x0010:_temp_eax
    mov $0xFE, %al
    mov %al, $0x0010:_except_num
    mov $0x0010:_temp_eax, %eax
    call _switchToKernel
