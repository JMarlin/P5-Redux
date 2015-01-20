.text

.globl _loadPageDirectory
_loadPageDirectory:
push %ebp
mov %esp, %ebp
mov 8(%esp), %eax
mov %eax, %cr3
mov %ebp, %esp
pop %ebp
ret

.globl _enablePaging
_enablePaging:
push %ebp
mov %esp, %ebp
mov %cr0, %eax
or $0x80000000, %eax
mov %eax, %cr0
mov %ebp, %esp
pop %ebp
ret

