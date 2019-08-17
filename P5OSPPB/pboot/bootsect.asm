;;Define the addresses of some external memory locations
%define V_BOOT_DRIVE 0x7e02

[org 0x7c00]
[bits 16]

;;Floppy boot header opens with a jump into the bootloader code and an extra
;;padding byte (which allows for a 16-bit jump vs 8)
jmp boot
;;db 0 ;Only required for padding if using jmp short

;;Most of the data below is calculated by the image generation script and then
;;passed in as assembler macros.
;================= FAT16 data ===================
db 'MYFILE  '            ;OEM name
dw SECTORSZ              ;Bytes per sector
db SECTPERCLUSTER        ;Sectors per cluster
dw RESSECT+1             ;Number of reserved sectors
db FATCOPIES             ;Copies of the FAT
dw ROOTENTRIES           ;Number of clusters in the root directory entry
dw TOTALSECT             ;Number of total sectors on disk
db 0xF8                  ;Type of disk (F0h = 1.44m floppy, F8h = hard disk)
dw SECTPERFAT            ;sectors per FAT
;================= FAT16 data ===================

;;A couple of debug strings
tststr db 'Bootsector Loaded.', 0
failstr db 'READ FAIL', 0
fail_reset_str db 'BAD RESET', 0
fail_lba_read_str db 'LBA ', 0
boot_drive db 0

;=========== DAP structure =============
align 16
stage_two_dap:
    db 0x10 ;DAP size
    db 0x00 ;unused/reserved
    dw RESSECT ;number of sectors to read
    dw 0x7e00 ;Offset to write data to
    dw 0x0000 ;Segment to write data to
    dq 0x01   ;LBA of first sector to read

;;Simple string print routine
printstr:

    pusha             ;Save the registers
    mov si, dx

    .top:
        mov bh, 0x0F
        mov al, [si]
        mov ah, 0x0E      ;Int 0x10, function 0xE: print char
        mov bl, 0
        int 0x10
        inc si            ;Get the next char and repeat if it wasn't 0
        cmp byte [si], 0
        je .done
        jmp .top
    .done:
        popa              ;Restore the registers
        ret

;;Simple byte print routine
printbyte:

    pusha
    mov dx, ax
    shr al, 4
    cmp al, 0x9
    jg hi_over_nine
    add al, 0x30
    jmp print_high 
    
hi_over_nine: 
    add al, 0x37
    
print_high:
    mov ah, 0x0E
    mov bl, 0
    int 0x10

    mov ax, dx
    and al, 0x0F
    cmp al, 0x9
    jg low_over_nine
    add al, 0x30
    jmp print_low
    
low_over_nine:
    add al, 0x37

print_low:
    mov ah, 0x0E
    mov bl, 0
    int 0x10

    popa
    ret 

;;BOOT CODE ENTRY POINT
boot:

    ;;Set up the stack at 0x7BFF (0x500 to 0x7FFFF is guaranteed RAM)
    ;;This will grow DOWN towards the vectors and BIOS memory
    mov ax, 0x7BFF
    mov sp, ax
    mov bp, ax

    ;;Make sure we're working out of the first segment
    ;;so all of our addressing is sane
    xor ax, ax
    mov ss, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ;;This makes sure CS gets set up without potentially throwing execution into
    ;;the wrong memory space
    jmp 0x0000:cs_set
    cs_set:

    ;;Capture the drive number we're booting on from the BIOS
    ;;NOTE: 0x7e02 will be a location at the beginning of the stage 2 code
    ;;where that code will be expecting to find this data
    mov [boot_drive], dl
    xor ax, ax
    mov al, dl
    call printbyte

    ;;Now that the basics are set up, print a confirmation
    mov dx, tststr
    call printstr

    ;;Reset drive
    mov ah, 0x0 ;Reset drive
    mov dl, [boot_drive]
    int 0x13
    jc failed_reset

    ;;Check to see if this computer supports extended functions
    mov ah, 0x41 ;Int 0x13, function 0x41: check for extended features
    mov dl, [boot_drive] ;Drive number should already be here, but why not
    mov bx, 0x55AA ;Magic number
    int 0x13
    jc legacy_load ;Carry is set if extensions do NOT exist

    ;Extended, LBA drive read
    mov ah, 0x42
    mov dl, [boot_drive]
    mov si, stage_two_dap
    int 0x13
    jc failed_lba_read
    jmp enter_stage_2

legacy_load:
    ;;Then all we really need to do, as the stage1, is read the stage2 from the
    ;;following sectors on the 'floppy'
    mov ah, 0x02     ;Int 0x13, function 2: read sectors
    mov al, RESSECT  ;Number of sectors to read. Need all of the stage2 sectors
    mov cx, 0x02     ;Cylinder 0 [7:6][15:8], sector 2 [5:0]
    mov bx, 0x7e00   ;Start of buffer (specifically es:bx)
    xor dh, dh       ;Head 0
    mov dl, [boot_drive] ;Drive number from the BIOS
    int 0x13
    jc failure       ;If carry is set, disk read failed

enter_stage_2:
    ;;If we got here, the sectors were read and we can jump into them
    ;;But first, pass the boot drive number
    mov dl, [boot_drive]
    mov [V_BOOT_DRIVE], dl
    jmp 0x7e00

    ;;On any failure, print failed message and hang
    failed_lba_read:
        mov edx, fail_lba_read_str
        call printstr
        jmp failure
    failed_reset:
	mov edx, fail_reset_str
        call printstr
        jmp hang
    failure:
        mov edx, failstr
        call printstr
    hang:
        jmp hang

;;Finally, we pad the rest of the code to 510 bytes (ie: one sector - 2) and
;;append the two-byte boot signature so that the BIOS knows were ttly srs
times 510-($-$$) db 0x0
dw 0xAA55
