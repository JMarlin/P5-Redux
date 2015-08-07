.extern _switchToKernel
.extern _in_kernel
.extern _except_num

.globl _pending_eoi
.globl _was_spurious
_was_spurious: .byte 0x0
_pending_eoi: .byte 0x0

.globl _spurious_handler
_spurious_handler:
    mov
    incb %ss:_in_kernel
    push %eax
    mov $0xFE, %al
    mov %al, %ss:_except_num
    mov $0x01, %al
    mov %al, _was_spurious
    pop %eax
    call _switchToKernel

.globl _handle_timerInt
_handle_timerInt:
    incb %ss:_in_kernel
    push %eax
    mov $0xFE, %al
    mov %al, %ss:_except_num
    xor %al, %al
    mov %al, _was_spurious
    pop %eax
    call _switchToKernel
