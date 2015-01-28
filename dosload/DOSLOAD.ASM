;DOSLOAD: A simple bootloader which allows booting a PROTICAL5
;image from within DOS
;This is basically PBOOT, but uses DOS to load the kernel into
;memory instead of loading sectors from a floppy

[BITS 16]
[ORG 0x100] ;This will be a COM executable

jmp main

kernel_name: DB "P5KERN.BIN",0
start_msg: DB "READING FILE...", 0xA, 0xD, "$"
run_msg: DB "LAUNCHING KERNEL...", 0xA, 0xD, "$"
file_msg: DB "COULDN'T OPEN FILE!", 0xA, 0xD, "$"
read_msg: DB "ERROR READING FILE!", 0xA, 0xD, "$"
move_msg: DB "MOVING IMAGE TO LOW MEMORY...", 0xA, 0xD, "$"
kernel_handle: DB 0x00, 0x00
byte_buf: DB 0x00
byte_counter: DW 0x00
ksize: DW 0x0000

main:
mov DX, 0xF000
mov ES, DX              ;We will be using the extra segment
                        ;to access the location the kernel will
                        ;be written to [0x0:0x1700]

mov ah, 0x9
mov dx, start_msg
int 0x21

mov AH, 0x3D            ;Dos INT 0x21 API - Open file
mov DX, kernel_name     ;ptr to file name
mov AL, 0xC0            ;Permissions, read-only exclusive
xor CL, CL              ;Just in case
INT 0x21

JC file_err

mov [kernel_handle], AX ;We're assuming this worked and
                        ;stashing the handle for now

mov dx, cs
mov ds, dx

readbyte:        
mov BX, [kernel_handle] ;Dos needs the handle in BX for this bit
mov AH, 0x3F            ;Dos INT 0x21 API - Read file
mov CX, 0x1             ;We're going to do this a byte at a time
mov DX, byte_buf        ;Pointer to the 'buffer'
INT 0x21

JC read_err

CMP AX, 0x0             ;If we read no bytes, leap to the boot code
JE boot

MOV AL, [byte_buf]      ;Otherwise, retrieve the read byte
MOV SI, [byte_counter]  ;Get the offset
MOV [0x1700+SI], AL  ;Store the byte into the boot area
INC SI                  ;Increase the offset
MOV [byte_counter], SI  ;And store it

JMP readbyte            ;Keep doing it until we've read the whole file

boot:
mov ah, 0x9
mov dx, move_msg
int 0x21

cli


;Move our completion code from seg to linear
xor ax, ax
mov es, ax
xor si, si
code_trans:
mov al, [enter_kernel+SI]
mov [ES:0x700+SI], al
inc si
cmp si, 0x200 ;Rough guess, I don't know the actual size of the code.
jne code_trans

;Move the kernel from segment memory to linear memory
xor ax, ax
mov es, ax
xor si, si
trans:
mov al, [0x1700+SI]
mov [ES:0x1700+SI], al
inc si
mov ax, [byte_counter]
cmp ax, si
jne trans


;here we will set up the GDT and jump into protected mode before
;leaping blindly and wildly into our loaded kernel (which should then
;be 32-bit)

mov si, 0x0
mv_gdt:
mov al, [gdtr+si]
mov [ES:0x600+si], al
inc si
cmp si, 31
jne mv_gdt 

lgdt [ES:0x600]     ;Load the GDT

;Programmatically insert the IDT (It's too big to fit in the sector)
;0x500 is the start of guaranteed free usable memory in the PC
;Manually insert an IRET at the expected memory location
mov al, 0xCF
mov [ES:0x670], al

mov si, 0x00    ;We'll loop 255 times
idt_load_top:
mov ax, 0x670
mov [0x500+si], ax
mov ax, 0x8
mov [0x502+si], ax
mov ax, 0x8E00
mov [0x504+si], ax
mov ax, 0x00
mov [0x506+si], ax

add si, 8
cmp si, 0x800
jne idt_load_top

xor si, si
mv_idt:
mov al, [idtr+si]
mov [ES:0x650+si], al
inc si
cmp si, 10
jne mv_idt

lidt [ES:0x650]     ;Load the dummy blank IDT

mov eax, cr0    ;Load the value in cr0
or ax, 0x0001   ;Set the PE bit, bit 0
mov cr0, eax    ;Update cr0 with the new value


;long-jump into the loaded kernel, specifying that it should
;run in the code-segment
jmp 0x08:0x700

[BITS 32]
enter_kernel:
;Make sure all of the data segment registers are set to
;the new GDT segment 0 (our GDT data segment)

mov eax, 0x10
mov ds, eax
mov es, eax
mov ss, eax
mov esp, 0x300000

;Enable A20
call wait_kbc
mov dx, 0x64
mov al, 0xAD
out dx, al
call wait_kbc
mov al, 0xD0
out dx, al
call wait_kbc2
mov dx, 0x60
in al, dx
mov bl, al
call wait_kbc
mov dx, 0x64
mov al, 0xD1
out dx, al
call wait_kbc
mov dx, 0x60
mov al, bl
or al, 0x2
out dx, al
call wait_kbc
mov al, 0xAE
mov dx, 0x64
out dx, al

testa20:
xor eax, eax
mov [0x101600], eax
mov eax, 0xFFFFFFFF
mov [0x1600], eax
mov ebx, [0x101600]
cmp ebx, eax
jne testdone
jmp testa20
testdone:

;one more round of kernel move
;now that we have high memory enabled, let's move the kernel there
xor esi, esi
trans2:
mov al, [0x1700+esi]
mov [0x100000+esi], al
inc si
xor eax, eax
mov ax, [byte_counter]
cmp eax, esi
jne trans2

;Dive into the kernel code that was loaded from the drive
;fuckit: jmp fuckit
jmp 0x08:0x100000

wait_kbc:
push dx
push ax
mov dx, 0x64
waitloop:
in al, dx
and al, 0x2
cmp al, 0
jne waitloop
pop ax
pop dx
ret

wait_kbc2:
push dx
push ax
mov dx, 0x64
waitloop2:
in al, dx
and al, 0x1
cmp al, 0x1
jne waitloop2
pop ax
pop dx
ret
[BITS 16]

idtr:
        idt_size: DW 0x800         ;We're going to initialize all of them
        idt_location: DD 0x500     ;Start of the table

fake_isr:
        IRET

gdtr:   ;The gdt descriptor which gives the GDT size and location
        gdt_limit: DW gdt_end - gdt_start - 1 ;Count of GDT entries
        gdt_location: DD 0x600 + (gdt_start - gdtr) ;Pointer to the table itself

gdt_start:

        null_segment:
        DD 0x0
        DD 0x0

        code_segment:   ;Segment 0x0008
        DB 0xFF, 0xFF    ;Limit 0:15
        DB 0x00, 0x00    ;Base 0:15
        DB 0x00         ;Base 16:23
        DB 0x9A         ;Access byte (Code segment, ring 0)
        DB 0xCF         ;Flags(0100b) and limit 16:19 (0xF)
        DB 0x00         ;Base 24:31

        data_segment:   ;Segment 0x0010
        DB 0xFF, 0xFF    ;Limit 0:15
        DB 0x00, 0x00    ;Base 0:15
        DB 0x00         ;Base 16:23
        DB 0x92         ;Access byte (Data segment, ring 0)
        DB 0xCF         ;Flags(0100b) and limit 16:19 (0xF)
        DB 0x00         ;Base 24:31

gdt_end:

file_err:
mov ah, 0x9
mov dx, file_msg
int 0x21
jmp quit

read_err:
mov ah, 0x9
mov dx, read_msg
int 0x21
jmp quit

quit:
mov ah, 0x4C
int 0x21

