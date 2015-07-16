.extern _kernelEntry
.extern _except_num
.extern _c_spurious_handler
.extern c_timer_handler
.extern _timer_int_ack
.extern t_counter
.extern _in_kernel
.extern needs_swap

.extern _old_esp
.extern _old_cr3
.extern _old_eip
.extern _old_eflags
.extern _old_eax
.extern _old_ecx
.extern _old_edx
.extern _old_ebx
.extern _old_ebp
.extern _old_esi
.extern _old_edi
.extern _old_es
.extern _old_cs
.extern _old_ss
.extern _old_ds
.extern _old_fs
.extern _old_gs
.extern _old_err

.globl _pending_eoi
_pending_eoi: .byte 0x0

.globl _timer_handler
_timer_handler:
/*TEST CODE*/
    /* pusha */

    /* stash the old DS from the interrupted code and install the ring0 DS
    xor %eax, %eax
    mov %ds, %ax
    push %eax
    mov %ss, %ax
    push %eax */
    mov $0x10, %ax
    mov %ax, %ss

    xor %eax, %eax
    mov %ds, %ax
    push %eax
    mov $0x10, %ax
    mov %ax, %ds

    call c_timer_handler

    /* restore the ring0 DS */
    pop %eax
    mov %ax, %ds

    /* popa */
    iret
/*REAL CODE BELOW*/
    push %ds
    push %eax
    mov $0x10, %ax
    mov %ax, %ds
    push %ebx
    mov t_counter, %eax
    inc %eax
    mov $2, %ebx
    cmp %eax, %ebx
    jg timer_reset

    mov %eax, t_counter
    mov $0x20, %al
    out %al, $0x20
    pop %ebx
    pop %eax
    pop %ds
    iret

 timer_reset:

    xor %eax, %eax
    mov %eax, t_counter

    inc %eax
    mov %al, needs_swap

    mov _in_kernel, %al
    xor %bl, %bl
    cmp %al, %bl
    je timer_to_kernel

    pop %ebx

    mov $0x20, %al
    out %al, $0x20

    pop %eax
    pop %ds
    iret

 timer_to_kernel:

    incb _in_kernel
    incb _pending_eoi

    mov $0xFE, %al
    mov %al, _except_num

    pop %ebx
    pop %eax
    pop %ds
    call _switchToKernel


.globl _spurious_handler
_spurious_handler:
    pusha
    /* call _c_spurious_handler */
    popa
    iret

.globl _irq_enter_kernel
irq_enter_kernel:


    cli
    push %eax
    mov $0xEF, %al
    mov %al, _except_num
    pop %eax
    call _switchToKernel
