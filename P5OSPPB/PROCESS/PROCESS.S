.extern _kernelEntry
.extern _except_num
.extern _timer_on
.extern _pending_eoi

.globl _stashCurrentState
.globl _switchToKernel
.globl _returnToProc

.globl _in_kernel
.globl _prc_is_super

.globl _old_esp
.globl _old_cr3
.globl _old_eip
.globl _old_eflags
.globl _old_eax
.globl _old_ecx
.globl _old_edx
.globl _old_ebx
.globl _old_ebp
.globl _old_esi
.globl _old_edi
.globl _old_es
.globl _old_cs
.globl _old_ss
.globl _old_ds
.globl _old_fs
.globl _old_gs
.globl _old_err

_old_esp: .int 0x0
_old_cr3: .int 0x0
_old_eip: .int 0x0
_old_eflags: .int 0x0
_old_eax: .int 0x0
_old_ecx: .int 0x0
_old_edx: .int 0x0
_old_ebx: .int 0x0
_old_ebp: .int 0x0
_old_esi: .int 0x0
_old_edi: .int 0x0
_old_es: .short 0x0
_old_cs: .short 0x0
_old_ss: .short 0x0
_old_ds: .short 0x0
_old_fs: .short 0x0
_old_gs: .short 0x0
_old_err: .int 0x0

_in_kernel: .byte 0x0
_prc_is_super: .byte 0x0

_switchToKernel:

    /* debug breakpoint */
    mov %eax, (0x1000)  

    /* Make sure the ESP gets restored regardless */
    /* of what the previous privilege level was */   
    /* NOTE: This isn't good enough. When we switch */
    /* the stack on a kernel mode process, we lose */
    /* the values that were popped onto the stack */
    /* during the INT execution. We need to safely */
    /* detect if we're in kernel mode very first thing */
    /* and, if we are, don't switch the stack until */
    /* we've gotten all the data we need. */    
    
    /* setup a quick temp ds for var access */
    /* push es first so we can restore it later */
    /* after using it for storing the original ds val */
    push %es 
    push %ds 
    push %eax 
    mov %ss, %ax
    mov %ax, %ds
    pop %eax  

    /* store segment regs, restoring the pushed */
    /* ds back into es */
    pop %es 
    mov %es, _old_ds
    pop %es 
    mov %es, _old_es
    mov %fs, _old_fs
    mov %gs, _old_gs 

    /* store general regs */
    mov %eax, _old_eax
    mov %ebx, _old_ebx
    mov %ecx, _old_ecx
    mov %edx, _old_edx
    mov %cr3, %eax
    mov %eax, _old_cr3
    mov %ebp, _old_ebp
    mov %esi, %eax
    mov %eax, _old_esi
    mov %edi, %eax
    mov %eax, _old_edi
    
    /* Discard the calling entry point's return */
    /* address from the top of the stack. We    */
    /* won't be going back that way             */
    pop %eax
    
    /* check the interrupt number to see if */
    /* we need to pop the error code or not */
    /* NOTE: for now, we're only going to */
    /* check for gpf */
    mov _except_num, %ax 
    cmp $0x8, %ax
    je pop_err
    cmp $0xA, %ax
    je pop_err
    cmp $0xB, %ax
    je pop_err
    cmp $0xC, %ax
    je pop_err
    cmp $0xD, %ax
    je pop_err
    cmp $0x11, %ax
    je pop_err
    jmp res_kentry
    
 pop_err:   
    pop %eax
    mov %eax, _old_err
    
 res_kentry:
    pop %eax
    mov %eax, _old_eip
    pop %eax
    mov %ax, _old_cs
    pop %eax
    mov %eax, _old_eflags
    
    /* Check _prc_is_super and, if it was set, don't get the esp or ss */
    /* In the future, it would probably be a great idea to retrieve the */
    /* proper kernel ESP instead of hijacking the process stack, but whatev */
    mov _prc_is_super, %eax
    cmp $0x0, %eax
    jne super_cont
        
    pop %eax
    mov %eax, _old_esp
    pop %eax
    mov %ax, _old_ss
    jmp notsuper_cont
    
 super_cont:
    mov %esp, _old_esp
    mov %ss, _old_ss
       
 notsuper_cont:   
    /* check to see if we have anything else on the stack */
    mov _old_eflags, %eax
    and $0x20000, %eax
    cmp $0x20000, %eax
    jne k_continue

 v86_enter:
    pop %eax
    mov %ax, _old_es    
    pop %eax
    mov %ax, _old_ds
    pop %eax
    mov %ax, _old_fs
    pop %eax
    mov %ax, _old_gs

 k_continue:   
   
    /* ensure that we're switched over to */
    /* the kernel stack */
    mov 0x002019A4, %esp  
 
    mov %ss, %ax
    mov %ax, %gs
    mov %ax, %fs
    mov %ax, %ds
    mov %ax, %es
    call _kernelEntry


_returnToProc:

    /* debug breakpoint */
    mov %eax, (0x1001)

    mov _old_eflags, %eax
    and $0x20000, %eax
    cmp $0x20000, %eax
    jne user
    
    xor %esp, %esp
    mov _old_ss, %sp 
    shl $4, %esp
    add _old_esp, %esp  
    mov $0x0, %eax    
    mov _old_gs, %ax
    push %eax
    mov _old_fs, %ax
    push %eax
    mov _old_ds, %ax
    push %eax
    mov _old_es, %ax
    push %eax
    mov _old_ss, %ax
    push %eax
    mov _old_esp, %eax
    push %eax
    mov _old_eflags, %eax
    push %eax
    mov $0x0, %eax
    mov _old_cs, %eax
    push %eax
    mov _old_eip, %eax
    push %eax
    mov %eax, _old_cr3
    mov %cr3, %eax
    mov _old_eax, %eax
    mov _old_ebx, %ebx
    mov _old_ecx, %ecx
    mov _old_edx, %edx
    mov _old_ebp, %ebp
    mov _old_esi, %esi
    mov _old_edi, %edi

    mov %eax, (0x1000)

    jmp kernreturn

    
 user:
    mov _old_esp, %esp 
    mov _old_ds, %ax
    mov %ax, %ds
    mov _old_es, %ax
    mov %ax, %es
    mov _old_fs, %ax
    mov %ax, %fs
    mov _old_gs, %ax
    mov %ax, %gs
    xor %eax, %eax
    mov _old_ss, %ax
    push %eax
    mov _old_esp, %eax
    push %eax 
    mov _old_eflags, %eax
    push %eax
    xor %eax, %eax
    mov _old_cs, %ax
    push %eax
    mov _old_eip, %eax
    push %eax
    mov %eax, _old_cr3
    mov %cr3, %eax
    mov _old_eax, %eax
    mov _old_ebx, %ebx
    mov _old_ecx, %ecx
    mov _old_edx, %edx
    mov _old_ebp, %ebp
    mov _old_esi, %esi
    mov _old_edi, %edi

 kernreturn:    
    
    push %eax
    mov _pending_eoi, %eax
    cmp $0, %eax
    jne kretfinish
   
    mov $0x20, %al
    out %al, $0x20
    decb _pending_eoi
        
 kretfinish:   
    pop %eax
    decb _in_kernel
    iret 
