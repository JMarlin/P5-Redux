.extern _switchToKernel
.extern _in_kernel
.extern _except_num

.globl _spurious_handler
_spurious_handler:
    pusha
    /* call _c_spurious_handler */
    popa
    iret

.globl _handle_timerInt
_handle_timerInt:
    incb %ss:_in_kernel
    push %eax
    mov $FE, %al
    mov %al, %ss:_except_num
    pop %eax
    call _switchToKernel
