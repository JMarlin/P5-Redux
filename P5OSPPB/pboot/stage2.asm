[org 0x7e00]
[bits 16]

;;Jump into the stage2 boot code
;;The 'short' forces this to be a 2-byte instruction, which we need so that the
;;following address is always at 0x7e02 where the stage1 code expects it to be
jmp short main

;;GLOBAL VARIABLES
boot_drive_number    db 0  ;Stage1 code will stash our drive number here
drive_head_count     db 0
drive_cylinder_count dw 0
drive_sector_count   db 0

;;And finally, we have the entry of the core of the boot code
main:

    ;;Set up for 32-bit protected mode

    ;;With all of that set up, go ahead and jump to the kernel loading code
    jmp 0x08:main_32

[bits 32]
main_32:

    ;;Tell the world we got to stage2
    mov edx, loaded
    call printstr

    ;;Get the CHS parameters of our boot drive
    call get_drive_params

    ;;Load the root directory sector and get the cluster # of P5KERN.BIN
    call get_kern_cluster

    ;;With the cluster number in ax, execute
    call load_from_cluster

    .hang jmp hang

;===============================================================================
; LBA2CHS
;===============================================================================
;;This function converts the LBA address passed via bx into CHS, formatted
into the registers as int 0x13 ah=2 expects
lba2chs:

;===============================================================================


;===============================================================================
; GET_DRIVE_PARAMS
;===============================================================================
;;This function grabs the parameters of the current drive from the BIOS and
;;nicely stores them for later use
get_drive_params:

    jmp start

    ;;Constant data used by this function
    .fail_message: `ERROR: Could not detect drive parameters.\n`

    .start:
        pusha                         ;Save the registers

        ;;Run the BIOS function (NOTE: logical, not physical params)
        mov ah, 0x08                  ;Int 0x13 ah=8: Get drive params
        mov dl, [boot_drive_number]   ;Get the drive passed by stage1
        xor di, di                    ;Apparently this screws with some BIOSes
        int 0x13
        jc failure                    ;A carry indicates failure

        ;;Format the returned data into our local variables
        ;;Store head count
        inc dh                        ;DH stores last head index, so +1 = count
        mov [drive_head_count], dh

        ;;Store cylinder count
        push cx                       ;Store CX while we decode the content
        mov cx, bx                    ;dupe it
        shr cx, 8                     ;Decode last cyl index at [7:6][15:8], +1
        and bx, 0xC0
        shl bx, 2
        or cx, bx
        inc cx
        mov [drive_cylinder_count], cx

        ;;Store sector count (no inc b/c sector index starts at 1, not 0)
        pop cx                        ;Retrieve the original cx value
        and cx, 0x3F                  ;Mask the last sector index at [5:0]
        mov [drive_sector_count], cl

        ;;Done
        popa
        ret

    ;;On failure, display the failure message and hang
    .failure:
        mov edx, fail_message
        call printstr
        .hang: jmp hang
;===============================================================================

;;Static data is stashed here
loaded db 'Jumped to second stage code', 0
yes db 'P5KERN.BIN found!', 0
no db 'P5KERN.BIN was not found on this disk.', 0
entrycount dd 0             ;the counter of entries in the root directory that aren't P5KERN.BIN.
datapointer dd 0x500        ;the pointer for reading disk data starting at a base of 0x500, where the root directory data will be read to. Important to make sure that this does NOT get overwritten by the stack
activesector db 4          ;the last sector to have bee read; track 2 sector 4 is the first sector of the root directory on this floppy
bdrive db 0
ce1 dw 0
ce2 dw 0
highbyte dw 0
midbyte dw 0
lowbyte dw 0
midbytelow dw 0
midbytehigh dw 0
ttrack db 0
tsector db 0
cluster dw 0
kernelsecoffset dw 0
fatbase dd 0
phys_offset dw 0
clusternum dw 0
thead db 0
spt dw 18
tracknum dw 0
headnum dw 2

pchar:
mov ah, 0x0E
mov bl, 0
int 0x10
ret

readsec:
mov ah, 0x0
int 0x13
mov ah, 0x02
mov al, 1
mov ch, 0
mov cl, [activesector]
mov bx, 0x500
mov dh, 1
mov dl, [bdrive]
int 0x13
;jc ufukd ;fail on carry
mov cl, [activesector]
add cl, 1
mov [activesector], cl
ret

printstr:
mov bh, 0x0F
mov al, [edx]
mov ah, 0x0E
mov bl, 0
int 0x10
add edx, 1
cmp byte [edx], 0
je printend
jmp printstr
printend:
ret

times 3301 db 0
