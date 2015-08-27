.globl _syscall_handler
.globl _syscall_number
.globl _syscall_param1
.globl _syscall_param2

.extern syscall_exec

_syscall_number:
    .byte 0x00, 0x00, 0x00, 0x00

_syscall_param1:
    .byte 0x00, 0x00, 0x00, 0x00

_syscall_param2:
    .byte 0x00, 0x00, 0x00, 0x00

_syscall_handler:
    mov %eax, _syscall_number
    mov %ebx, _syscall_param1
    mov %ecx, _syscall_param2
    call syscall_exec
    iret
