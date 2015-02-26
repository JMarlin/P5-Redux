.extern _switchToKernel
.extern _except_num
.extern _c_timer_handler
.extern _c_spurious_handler

.globl _timer_handler
_timer_handler: 
    pusha
    call _c_timer_handler    
    popa
    iret

.globl _spurious_handler
_spurious_handler:
    pusha
    /*call _c_spurious_handler*/
    popa
    iret
    
    cli
    push %eax 
    mov $0xEF, %al
    mov %al, _except_num
    pop %eax 
    call _switchToKernel
    