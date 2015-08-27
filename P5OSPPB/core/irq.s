.globl irq_handler_1
irq_handler_1:
    incb %ss:_in_kernel
    push %eax
    movb $0, %ss:_has_error_code /*Has no error code*/
    mov $0xE1, %al
    mov %al, %ss:_except_num
    xor %al, %al
    mov %al, %ss:_was_spurious
    pop %eax
    jmp _switchToKernel

.globl irq_handler_2
irq_handler_2:
    incb %ss:_in_kernel
    push %eax
    movb $0, %ss:_has_error_code /*Has no error code*/
    mov $0xE2, %al
    mov %al, %ss:_except_num
    xor %al, %al
    mov %al, %ss:_was_spurious
    pop %eax
    jmp _switchToKernel

.globl irq_handler_3
irq_handler_3:
    incb %ss:_in_kernel
    push %eax
    movb $0, %ss:_has_error_code /*Has no error code*/
    mov $0xE3, %al
    mov %al, %ss:_except_num
    xor %al, %al
    mov %al, %ss:_was_spurious
    pop %eax
    jmp _switchToKernel

.globl irq_handler_4
    irq_handler_4:
    incb %ss:_in_kernel
    push %eax
    movb $0, %ss:_has_error_code /*Has no error code*/
    mov $0xE4, %al
    mov %al, %ss:_except_num
    xor %al, %al
    mov %al, %ss:_was_spurious
    pop %eax
    jmp _switchToKernel

.globl irq_handler_5
    irq_handler_5:
    incb %ss:_in_kernel
    push %eax
    movb $0, %ss:_has_error_code /*Has no error code*/
    mov $0xE5, %al
    mov %al, %ss:_except_num
    xor %al, %al
    mov %al, %ss:_was_spurious
    pop %eax
    jmp _switchToKernel

.globl irq_handler_6
    irq_handler_6:
    incb %ss:_in_kernel
    push %eax
    movb $0, %ss:_has_error_code /*Has no error code*/
    mov $0xE6, %al
    mov %al, %ss:_except_num
    xor %al, %al
    mov %al, %ss:_was_spurious
    pop %eax
    jmp _switchToKernel

.globl irq_handler_7
    irq_handler_7:
    incb %ss:_in_kernel
    push %eax
    movb $0, %ss:_has_error_code /*Has no error code*/
    mov $0xE7, %al
    mov %al, %ss:_except_num
    xor %al, %al
    mov %al, %ss:_was_spurious
    pop %eax
    jmp _switchToKernel

.globl irq_handler_8
irq_handler_8:
    incb %ss:_in_kernel
    push %eax
    movb $0, %ss:_has_error_code /*Has no error code*/
    mov $0xE8, %al
    mov %al, %ss:_except_num
    xor %al, %al
    mov %al, %ss:_was_spurious
    pop %eax
    jmp _switchToKernel

.globl irq_handler_9
irq_handler_9:
    incb %ss:_in_kernel
    push %eax
    movb $0, %ss:_has_error_code /*Has no error code*/
    mov $0xE9, %al
    mov %al, %ss:_except_num
    xor %al, %al
    mov %al, %ss:_was_spurious
    pop %eax
    jmp _switchToKernel

.globl irq_handler_10
irq_handler_10:
    incb %ss:_in_kernel
    push %eax
    movb $0, %ss:_has_error_code /*Has no error code*/
    mov $0xEA, %al
    mov %al, %ss:_except_num
    xor %al, %al
    mov %al, %ss:_was_spurious
    pop %eax
    jmp _switchToKernel


.globl irq_handler_11
irq_handler_11:
    incb %ss:_in_kernel
    push %eax
    movb $0, %ss:_has_error_code /*Has no error code*/
    mov $0xEB, %al
    mov %al, %ss:_except_num
    xor %al, %al
    mov %al, %ss:_was_spurious
    pop %eax
    jmp _switchToKernel

.globl irq_handler_12
irq_handler_12:
    incb %ss:_in_kernel
    push %eax
    movb $0, %ss:_has_error_code /*Has no error code*/
    mov $0xEC, %al
    mov %al, %ss:_except_num
    xor %al, %al
    mov %al, %ss:_was_spurious
    pop %eax
    jmp _switchToKernel

.globl irq_handler_13
irq_handler_13:
    incb %ss:_in_kernel
    push %eax
    movb $0, %ss:_has_error_code /*Has no error code*/
    mov $0xED, %al
    mov %al, %ss:_except_num
    xor %al, %al
    mov %al, %ss:_was_spurious
    pop %eax
    jmp _switchToKernel

.globl irq_handler_14
irq_handler_14:
    incb %ss:_in_kernel
    push %eax
    movb $0, %ss:_has_error_code /*Has no error code*/
    mov $0xEE, %al
    mov %al, %ss:_except_num
    xor %al, %al
    mov %al, %ss:_was_spurious
    pop %eax
    jmp _switchToKernel

.globl irq_handler_15
irq_handler_15:
    incb %ss:_in_kernel
    push %eax
    movb $0, %ss:_has_error_code /*Has no error code*/
    mov $0xEF, %al
    mov %al, %ss:_except_num
    xor %al, %al
    mov %al, %ss:_was_spurious
    pop %eax
    jmp _switchToKernel
