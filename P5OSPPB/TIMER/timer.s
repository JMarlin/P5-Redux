.extern _switchToKernel
.extern _except_num
.extern _c_timer_handler

.globl _timer_handler
_timer_handler: 
    cli
    pusha
    call _c_timer_handler    
    popa
    sti
    iret

    cli
    push %eax 
    mov $0xEF, %al
    mov %al, _except_num
    pop %eax 
    call _switchToKernel
    