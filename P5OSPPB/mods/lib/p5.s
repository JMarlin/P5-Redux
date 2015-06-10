.globl _dest
.globl _command
.globl _payload
.globl _retval

_dest: .int 0x0
_command: .int 0x0
_payload: .int 0x0
_retval: .int 0x0

.globl _asm_send_msg
_asm_send_msg:
    pusha
    mov _dest, %ebx
    mov _command, %ecx
    mov _payload, %edx
    mov $0x1, %eax
    int $0xFF
    popa
    ret

.globl _asm_get_msg
_asm_get_msg:
    pusha
    mov $0x2, %eax
    mov $0x0, %ebx
    int $0xFF
    mov %eax, _retval
    mov %ebx, _dest
    mov %ecx, _command
    mov %edx, _payload
    popa
    ret

.globl _asm_get_msg_from
_asm_get_msg_from:
    pusha
    mov $0x2, %eax
    mov $0xFFFFFFFF, %ebx
    mov _dest, %ecx
    mov _command, %edx
    int $0xFF
    mov %eax, _retval
    mov %ebx, _dest
    mov %ecx, _command
    mov %edx, _payload
    popa
    ret
