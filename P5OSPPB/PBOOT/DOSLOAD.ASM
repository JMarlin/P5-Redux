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
kernel_handle: DW 0x00
byte_buf: DB 0x00
byte_counter: DW 0x00

main:
mov DX, 0x0
mov ES, DX              ;We will be using the extra segment
                        ;to access the location the kernel will
                        ;be written to [0x0:0x1700]

mov AH, 0x3D            ;Dos INT 0x21 API - Open file
mov DX, kernel_name     ;ptr to file name
mov AL, 0xC0            ;Permissions, read-only exclusive
xor CL, CL              ;Just in case
INT 0x21

mov [kernel_handle], AX ;We're assuming this worked and
                        ;stashing the handle for now
mov ah, 0x9
mov dx, start_msg
int 0x21

readbyte:        
mov BX, [kernel_handle] ;Dos needs the handle in BX for this bit
mov AH, 0x3F            ;Dos INT 0x21 API - Read file
mov CX, 0x1             ;We're going to do this a byte at a time
mov DX, byte_buf        ;Pointer to the 'buffer'
INT 0x21

CMP AX, 0x0             ;If we read no bytes, leap to the boot code
JE boot

MOV AL, [byte_buf]      ;Otherwise, retrieve the read byte
MOV SI, [byte_counter]  ;Get the offset
MOV [ES:0x1700+SI], AL  ;Store the byte into the boot area
INC SI                  ;Increase the offset
MOV [byte_counter], SI  ;And store it

JMP readbyte            ;Keep doing it until we've read the whole file

boot:
;here we will set up the GDT and jump into protected mode before
;leaping blindly and wildly into our loaded kernel (which should then
;be 32-bit)
cli             ;disable interrupts
lgdt [gdtr]     ;Load the GDT
mov eax, cr0    ;Load the value in cr0
or ax, 0x0001   ;Set the PE bit, bit 0
mov cr0, eax    ;Update cr0 with the new value

;long-jump into the loaded kernel, specifying that it should
;run in the code-segment
jmp 0x08:enter_kernel

[BITS 32]
enter_kernel:
;Make sure all of the data segment registers are set to
;the new GDT segment 0 (our GDT data segment)
mov eax, 0x10
mov ds, eax
mov es, eax
mov ss, eax
mov esp, 0x90000

;Programmatically insert the IDT (It's too big to fit in the sector)
;0x500 is the start of guaranteed free usable memory in the PC
mov si, 0x00    ;We'll loop 255 times
idt_load_top:
mov ax, fake_isr
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

lidt [idtr]     ;Load the dummy blank IDT

;Dive into the kernel code that was loaded from the drive
jmp 0x1700

[BITS 16]

vertop db 10,13,'Kernel ',0
verbot db ' Loaded.',10,13,'Booting...',0

idtr:
        idt_size: DW 0x800         ;We're going to initialize all of them
        idt_location: DD 0x500     ;Start of the table

fake_isr:
        IRET

gdtr:   ;The gdt descriptor which gives the GDT size and location
        gdt_limit: DW gdt_end - gdt_start - 1 ;Count of GDT entries
        gdt_location: DD gdt_start ;Pointer to the table itself

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


JMP 0x0:0x1700
