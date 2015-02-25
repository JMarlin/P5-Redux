.extern _switchToKernel
.extern _except_num

.globl _timer_handler
_timer_handler: 
    cli
    push %eax 
    mov $0xEF, %al
    mov %al, _except_num
    pop %eax 
    call _switchToKernel