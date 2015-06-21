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
;    jmp 0x08:main_32

;[bits 32]
;main_32:

    ;;Tell the world we got to stage2
    mov dx, loaded
    call printstr

    ;;Get the CHS parameters of our boot drive
    call get_drive_params

    ;;Display the drive parameters
    ;;Drive number
    mov dx, drive_num_str
    call printstr
    mov al, [boot_drive_number]
    call print_hex_char

    ;;Head count
    mov dx, head_count_str
    call printstr
    mov al, [drive_head_count]
    call print_hex_char

    ;;Cylinder count
    mov dx, cylinder_count_str
    call printstr
    mov ax, [drive_cylinder_count]
    call print_hex_word

    ;;Sector count
    mov dx, sector_count_str
    call printstr
    mov al, [drive_sector_count]
    call print_hex_char

    ;;Load the root directory sector and get the cluster # of P5KERN.BIN
    ;call get_kern_cluster

    ;;With the cluster number in ax, execute
    ;call load_from_cluster

    .hang jmp .hang


;===============================================================================
; GET_DRIVE_PARAMS
;===============================================================================
;;This function grabs the parameters of the current drive from the BIOS and
;;nicely stores them for later use
get_drive_params:

    jmp .start

    ;;Constant data used by this function
    .fail_message: db `ERROR: Could not detect drive parameters.\n`

    .start:
        pusha                         ;Save the registers

        ;;Run the BIOS function (NOTE: logical, not physical params)
        mov ah, 0x08                  ;Int 0x13 ah=8: Get drive params
        mov dl, [boot_drive_number]   ;Get the drive passed by stage1
        xor di, di                    ;Apparently this screws with some BIOSes
        int 0x13
        jc .failure                    ;A carry indicates failure

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
        mov edx, .fail_message
        call printstr
        .hang: jmp .hang


;===============================================================================
; PRINTCHAR
;===============================================================================
;;Prints the char in al
printchar:

    push bx
    push ax
    mov bh, 0x0F
    mov ah, 0x0E
    mov bl, 0
    int 0x10
    pop ax
    pop bx
    ret


;===============================================================================
; PRINTSTR
;===============================================================================
;;Prints the ASCIIZ string pointed to by edx
printstr:

    push ax
    mov si, dx

    .top:
        mov al, [si]
        call printchar
        inc si
        cmp byte [si], 0
        je .end
        jmp .top

    .end:
        pop ax
        ret

;===============================================================================
; HEX2CHAR
;===============================================================================
;; Converts the low nybble in al to its corresponding ASCII character and
;; returns that value in al
hex2char:

    ;;See if the value is over ten
    and al, 0xF    ;Isolate the low nybble
    cmp al, 0xA
    jl .under_ten

    ;;If the value is over nine, subtract ten, add 'A' and return
    sub al, 0xA
    add al, 'A'
    ret

    ;;If the value is under ten, simply add '0' and return
    .under_ten:
        add al, '0'
        ret


;===============================================================================
; PRINT_HEX_CHAR
;===============================================================================
;; Takes a value in al and prints its hex value to the screen
print_hex_char:

    push ax
    push ax
    shr al, 4
    call hex2char
    call printchar
    pop ax
    call hex2char
    call printchar
    pop ax
    ret


;===============================================================================
; PRINT_HEX_WORD
;===============================================================================
;; Takes a value in ax and prints its hex value to the screen
print_hex_word:

    push ax
    push ax
    mov al, ah
    call print_hex_char
    pop ax
    call print_hex_char
    pop ax
    ret

;===============================================================================


;===============================================================================
; CLUSTER_TO_CSH
;===============================================================================
;; Takes a cluster number in ax and convert it into csh values formatted for
;;read_sector (sector# in al, head# in ah, and cylinder number in bx
;;LBA numbering starts with all sectors in a cylinder, then switches heads and
;;resets sectors from zero, then switches cylinders and resets heads and
;;sectors. Eg: Cluster number 53 with 16spt, 2 heads will be:
;;Calculate: 
;;           lba = (cluster_no * sectors_per_cluster)              
;;               + reserved_sectors   
;;               + fat_count*fat_sz
;;               + ceil((dir_ent*32)/512)
;;               - 2
;;           sector_no = (lba % spt) + 1 ((53%16)+1 = 6)
;;           cylinder_no = lba / (spt * head_count) (53/32 = 1)
;;           head_no = (lba / spt) % head_count ((53/16)%2 = 1)
;;     CHS = 1, 1, 6
cluster_to_csh:
  
  
  ret
;===============================================================================


;===============================================================================
; GET_CLUSTER_FAT_ENTRY
;===============================================================================
;; Gets a cluster number and finds its next-value FAT entry value
get_cluster_fat_entry:

    mov ax, [clusternum]
    add ax, 2
    mov cx, 2
    mov edx, 0
    div cx
    sub ax, 1
    mov bx, ax
    add ax, bx
    add ax, bx
    mov [phys_offset], ax
    mov bx, [phys_offset]
    mov ah, 0
    mov al, [bx+0x500]
    mov [lowbyte], ax
    mov bx, [phys_offset]
    mov ah, 0
    mov al, [bx+0x501]
    mov [midbyte], ax
    mov bx, [phys_offset]
    mov ah, 0
    mov al, [bx+0x502]
    mov [highbyte], ax
    mov ax, [midbyte]
    and ax, 0x000f
    mov [midbytelow], ax
    mov ax, [midbyte]
    and ax, 0x00f0
    mov [midbytehigh], ax
    mov ax, [highbyte]
    shl ax, 8
    mov [highbyte], ax
    mov ax, [midbytelow]
    shl ax, 8
    mov [midbytelow], ax
    mov ax, [highbyte]
    mov bx, [midbytehigh]
    add ax, bx
    shr ax, 4
    mov [ce2], ax
    mov ax, [lowbyte]
    mov bx, [midbytelow]
    add ax, bx
    mov [ce1], ax
    mov ax, [clusternum]
    mov cx, 2
    mov edx, 0
    div cx
    mov cx, dx
    jcxz clusterone
    jmp clustertwo
    
    .clusterone:
        mov ax, [ce1]
        mov [cluster], ax
        jmp toout
    
    .clustertwo:
        mov ax, [ce2]
        mov [cluster], ax
        
    .toout:
        ret
;===============================================================================


;===============================================================================
; READ_CLUSTER
;===============================================================================
;; MAKE THIS WORK
;; Should take a cluster number, translate it to CHS, and then run it through
;; read_sector
;===============================================================================


;===============================================================================
; READ_SECTOR
;===============================================================================
;; Gets a CHS value and a buffer address and reads a sector there
read_sector:

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
    jc .error
    mov cl, [activesector]
    add cl, 1
    mov [activesector], cl
    ret
;===============================================================================

;;Static data is stashed here
loaded db `Jumped to second stage code`, 0
drive_num_str db `\r\nBoot drive number: 0x`, 0
head_count_str db `\r\nHead count: 0x`, 0
cylinder_count_str db `\r\nCylinder count: 0x`, 0
sector_count_str db `\r\nSectors per track: 0x`, 0
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

times 3301 db 0
