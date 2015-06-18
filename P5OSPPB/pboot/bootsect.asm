[ORG 0x7c00]
[BITS 16]

;;Floppy boot header opens with a jump into the bootloader code and an extra
;;padding byte (which allows for a 16-bit jump vs 8)
jmp short boot
db 0

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
db 0xF0                  ;Type of disk (F0h = 1.44m floppy, F8h = hard disk)
dw SECTPERFAT            ;sectors per FAT (192 is large, but allows for 32-meg worth of clusters)
;================= FAT16 data ===================

;;A couple of debug strings
tststr db 'Bootsector Loaded.', 0
failstr db `FAIL`, 0

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

;;BOOT CODE ENTRY POINT
boot:

    ;;Capture the drive number we're booting on from the BIOS
    ;;NOTE: 0x7e02 will be a location at the beginning of the stage 2 code
    ;;where that code will be expecting to find this data
    mov [0x7e02], dl

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

    ;;Now that the basics are set up, print a confirmation
    mov dx, tststr
    call printstr

    ;;Then all we really need to do, as the stage1, is read the stage2 from the
    ;;following sectors on the 'floppy'
    mov ah, 0x0      ;Reset drive
    int 0x13
    mov ah, 0x02     ;Int 0x13, function 2: read sectors
    mov al, RESSECT  ;Number of sectors to read. Need all of the stage2 sectors
    mov cx, 0x02     ;Cylinder 0 [7:6][15:8], sector 2 [5:0]
    mov bx, 0x7e00   ;Start of buffer (specifically es:bx)
    xor dh, dh       ;Head 0
    mov dl, [0x7e02] ;Drive number from the BIOS
    int 0x13
    jc failure       ;If carry is set, disk read failed

    ;;If we got here, the sectors were read and we can jump into them
    jmp 0x7e00

    ;;On any failure, print failed message and hang
    failure:
        mov edx, failstr
        call printstr
        hang:
        jmp hang

;;Finally, we pad the rest of the code to 510 bytes (ie: one sector - 2) and
;;append the two-byte boot signature so that the BIOS knows were ttly srs
times 510-($-$$) db 0x0
dw 0xAA55
