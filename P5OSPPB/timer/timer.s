.extern _switchToKernel
.extern _in_kernel
.extern _except_num
.extern _has_error_code
.extern c_calc_mips

.globl _pending_eoi
.globl _was_spurious
_was_spurious: .byte 0x0
_pending_eoi: .byte 0x0

.globl _spurious_handler
_spurious_handler:
    incb %ss:_in_kernel
    push %eax
    movb $0, %ss:_has_error_code /*Has no error code*/
    mov $0xFE, %al
    mov %al, %ss:_except_num
    mov $0x01, %al
    mov %al, %ss:_was_spurious
    pop %eax
    jmp _switchToKernel

.globl _handle_timerInt
_handle_timerInt:
    incb %ss:_in_kernel 
    push %eax
    movb $0, %ss:_has_error_code /*Has no error code*/
    mov $0xFE, %al
    mov %al, %ss:_except_num
    xor %al, %al
    mov %al, %ss:_was_spurious
    pop %eax
    jmp _switchToKernel

/*Need to make sure we dump interrupt stacks in the future*/
.globl _calc_mips
_calc_mips:
    cli
    jmp c_calc_mips

.globl _mips_counter
_mips_counter: .int 0x0     
.globl _mips_loop
_mips_loop:
    mov $0, %eax
    mov %eax, _mips_counter
    sti
    mips_top:
        add $1, %eax
        mov %eax, _mips_counter
        xchg %eax, %eax /* NOP */
        jmp mips_top
    
