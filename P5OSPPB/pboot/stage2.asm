;;Define the addresses of some external variables
;;Most of these are values from the FAT header
%define V_SECTORSZ 0x7c08         ;word
%define V_SECTPERCLUSTER 0x7c0a   ;byte
%define V_RESSECT 0x7c0b          ;word
%define V_FATCOPIES 0x7c0d        ;byte
%define V_ROOTENTRIES 0x7c0e      ;word
%define V_TOTALSECT 0x7c10        ;word
%define V_SECTPERFAT 0x7c12       ;word

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

    ;;Tell the world we got to stage2
    mov dx, .loaded_str
    call printstr

    ;;Get the CHS parameters of our boot drive
    call get_drive_params

    ;;Display the drive parameters
    ;;Drive number
    mov dx, .drive_num_str
    call printstr
    mov al, [boot_drive_number]
    call print_hex_char
    ;;Head count
    mov dx, .head_count_str
    call printstr
    mov al, [drive_head_count]
    call print_hex_char
    ;;Cylinder count
    mov dx, .cylinder_count_str
    call printstr
    mov ax, [drive_cylinder_count]
    call print_hex_word
    ;;Sector count
    mov dx, .sector_count_str
    call printstr
    mov al, [drive_sector_count]
    call print_hex_char

    ;;Print a return
    mov al, '\r'
    call printchar
    mov al, '\n'
    call printchar

    ;;Load the root directory sector and get the cluster # of P5KERN.BIN
    mov dx, .searching_str   ;Print a notice
    call printstr
    ;;Look up
    call get_kern_cluster
    ;;Make sure the file was found
    cmp ax, 0
    jne .chk_filesz
    mov dx, .not_found_str
    call printstr
    jmp .hang

    .chk_filesz:
    ;;Check to see if the file is too big (can be max 7 segments, about 450k)
    ;;(most significant word of filesize is in cx)
    cmp cx, 7
    jl .cnt_load_file
    mov dx, .too_big_str
    call printstr
    jmp .hang

    .cnt_load_file:
    mov dx, .found_str
    call printstr
    ;;With cluster number in ax, load it starting at 0x1000:0x0000 (0x10000)
    mov es, 0x1000     ;Start address is at es:0x0
    call load_from_cluster

    ;;File is loaded, now we can focus on setting up protected mode
    mov dx, .read_str
    call printstr

    ;;Set up for 32-bit protected mode

    ;;With all of that set up, go ahead and jump to the kernel loading code
;    jmp 0x08:main_32

    ;;Hang loop
    .hang jmp .hang

    ;;Local constants
    .read_str db `loaded.\r\nBooting.`, 0
    .found_str db `found\r\nLoading into low memory...`,0
    .searching_str db `Searching for kernel image...`,0
    .not_found_str db `Kernel not found on the boot disk.`, 0
    .too_big_str db `kernel exceeds 450kb.`, 0
    .loaded_str db `\r\nJumped to second stage code`, 0
    .drive_num_str db `\r\nBoot drive number: 0x`, 0
    .head_count_str db `\r\nHead count: 0x`, 0
    .cylinder_count_str db `\r\nCylinder count: 0x`, 0
    .sector_count_str db `\r\nSectors per track: 0x`, 0
;[bits 32]
;main_32:

    ;;Enable A20
    ;;Move kernel to high memory
    ;;Do dat bootin

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
; LBA_TO_CSH
;===============================================================================
;;Takes a zero-indexed LBA value in ax and spits out CHS values as expected by
;;read_sector (sector# in al, head# in ah, and cylinder number in bx)
;;Calculate the sector number
lba_to_csh:

    push dx

    push ax    ;Save the LBA number to the stack because it's gonna be clobbered
    xor dx, dx
    div [drive_sector_count]
    inc dx
    mov bl, dl ;Save the low 8 bits of the remainder for output

    ;;Calculate the cylinder number
    pop ax     ;Restore the LBA value from the stack
    push ax    ;And dupe it again as it's about to get clobbered again
    mov bx, [drive_sector_count]
    mul bx, [drive_head_count]   ;bx = spt * heads
    xor dx, dx
    div bx
    mov bx, ax                  ;bx = lba / (spt*heads)

    ;;Calculate the head number
    pop ax     ;Restore LBA again
    xor dx, dx
    div [drive_sector_count] ;ax = lba/spt
    xor dx, dx ;Clear any remainder
    div [drive_head_count] ;dx = (lba/spt) % head_count

    ;;Finally, repackage the registers as expected, pop clobbered values, return
    mov ah, dl ;ah = head
    mov al, bl ;al = sect
    mov bx, cx ;bx = cyl
    pop dx
    ret
;===============================================================================

;===============================================================================
; CLUSTER_TO_CSH
;===============================================================================
;; Takes a cluster number in ax and convert it into csh values formatted for
;;read_sector (sector# in al, head# in ah, and cylinder number in bx)
;;LBA numbering starts with all sectors in a cylinder, then switches heads and
;;resets sectors from zero, then switches cylinders and resets heads and
;;sectors. Eg: Cluster number 53 with 16spt, 2 heads will be:
;;Calculate:
;;           lba = (cluster_no * sectors_per_cluster)
;;               + reserved_sectors
;;               + fat_count*fat_sz
;;               + ceil((dir_ent*32)/512)
;;               - 2 //sectors 0 and 1 reserved
;;               - 1 //lba is 0 indexed
;;           sector_no = (lba % spt) + 1 ((53%16)+1 = 6)
;;           cylinder_no = lba / (spt * head_count) (53/32 = 1)
;;           head_no = (lba / spt) % head_count ((53/16)%2 = 1)
;;     CHS = 1, 1, 6
cluster_to_csh:

  ;;Push clobbered regs
  push dx
  push cx

  ;;Calculate LBA from clusternum
  mov dl, [V_SECTPERCLUSTER]
  mul ax, dl
  mov dx, [V_RESSECT]
  add ax, dx
  mov dx, [V_FATCOPIES]
  mov bx, [V_SECTPERFAT]
  mul dx, bx
  add ax, dx
  mov dx, [V_ROOTENTRIES]
  mul dx, 32
  push ax
  mov ax, dx
  xor dx, dx
  div 512
  cmp dx, 0
  jne .noadd
  inc ax
  .noadd:
  mov dx, ax
  pop ax
  sub ax, 3 ;AX now contains the LBA calculation from above

  ;;Convert the calculated lba into CSH values
  call lba_to_csh

  pop cx
  pop dx
  ret
;===============================================================================


;===============================================================================
; READ_SECTOR
;===============================================================================
;;Takes a CHS value in the form al=sector ah=head bx=cylinder and reads that
;;sector into the address specified by es:dx
read_sector:

    ;;Save clobbered registers
    push cx
    push dx

    ;;Package heads into CX in odd BIOS 76543210|98xxxxxx format
    mov cx, bx
    shl cx, 0xa  ;00000098|76543210 -> 76543210|00000000
    mov cl, bh   ;76543210|00000098
    shl cl, 6    ;76543210|98000000
    ;;And then package low six bits of sector into the rest of CL
    and al, 0x3f ;00ssssss
    or cl, al    ;76543210|98ssssss
    ;;Move the buffer pointer into the register the BIOS expects
    mov bx, dx
    ;;Move head to the register the BIOS expects
    mov dh, ah
    ;;Set the drive number to our boot drive
    mov dl, [boot_drive_number]
    ;;Read one sector
    mov al, 1
    ;;Set the function and make the BIOS call
    mov ah, 0x02
    int 0x13
    ;;Do a fail-hang unless the read succeeded
    jnc .success
    mov dx, [.fail_msg]
    call printstr
    .failhang: jmp .failhang

    .success:
        pop dx
        pop cx
        ret

    ;;Local constants
    .fail_msg: db "Failure reading sector."
;===============================================================================


;===============================================================================
; READ_ROOT_SECTOR
;===============================================================================
;;Does some math to figure out where the FAT root directory sector is and then
;;read that sector into RAM starting at 0x500
get_root_sector:

    pusha

    ;;Add the reserved sectors and the fat sectors to get the LBA number
    mov ax, [V_SECTPERFAT]
    xor bx, bx
    mov bl, [V_FATCOPIES]
    mul ax, bx
    add ax, [V_RESSECT]

    ;;Convert the LBA sector to a CHS value for read_sector, then read it
    call lba_to_chs
    xor es, es         ;Store at es:dx
    mov dx, 0x500
    call read_sector

    ;;Sector is in memory, clean up and exit
    popa
    ret
;===============================================================================


;===============================================================================
; GET_KERN_CLUSTER
;===============================================================================
;;Finds the offset to the root directory sector based on the FAT header info,
;;then reads that sector into memory and scans the entry names for P5KERN.BIN
;;and finally returns the beginning cluster number found in that directory
;;entry in AX (zero on failure) and file size in cx:dx
get_kern_cluster:

    push bx
    push di

    ;;Read the root directory entry into 0x0:0x500
    call read_root_sector

    xor bx, bx ;Gonna use bx as our index into the root entries

    ;;Loop through the entries
    .entry_loop

        cmp bx, 0x200 ;ie 512 ie the size of a sector
        je .entry_break ;wend
        ;Gonna use di as our string index
        xor di, di

        ;;Loop over the entry name string
        .string_loop:

            mov al, [bx+di+0x500] ;Get current directory character in al
            push bx               ;switch to our string's base
            mov bx, .kernel_name
            cmp al, [bx+di]       ;Go to the next entry if they don't match
            jne .string_break
            pop bx                ;Switch back to entry base
            inc di                ;index++
            cmp di, 11            ;If we haven't matched all, loop
            jne .string_loop

            ;;We found it! now we just need to get the cluster in AX and ret
            xor di, di            ;don't need the index anymore
            mov ax, [bx+di+0x1A]  ;Cluster number is at offset 0x1A in entry
            mov dx, [bx+di+0x1C]  ;File size starts after it
            mov cx, [bx+di+0x1E]
            call .done
        .string_break:

        pop bx       ;switch back to the incrementing base
        add bx, 0x20 ;ie 32 ie the size of a directory entry
        jmp .entry_loop
    .entry_break:

        ;;We failed to find a matching entry, so we return all zeros
        xor ax, ax
        xor cx, cx
        xor dx, dx

    .done:

        pop di
        pop bx
        ret

    ;;Local constants
    .kernel_name: db "P5KERN  BIN" ;The kernel name as an 8.3 string
;===============================================================================


;===============================================================================
; LOAD_FROM_CLUSTER
;===============================================================================
;;Takes a cluster number in ax and a starting address in es:bx and reads the
;;file starting at that cluster into memory
load_from_cluster:

    pusha

    xor dx, dx ;Always starts on a segment boundary

    .sector_read_loop:

        ;Exit if cluster number is EOF
        cmp ax, 0xFFEF
        jg .sector_read_loop_exit

        ;Read cluster ax into es:dx
        push ax
        call cluster_to_chs
        call read_sector

        ;Increase our pointers
        add dx, 0x200 ;Read 512 bytes so scoot forward as many
        jnc .proceed  ;Unless there was no overflow, we move to the next seg
        add es, 0x1000
        xor dx, dx    ;just to be safe

        .proceed
            ;Get the next cluster number
            pop ax    ;restore the current cluster number
            call get_next_cluster ;Puts the new number in ax

    .sector_read_loop_exit:

    popa
    ret
;===============================================================================


;===============================================================================
; GET_NEXT_CLUSTER
;===============================================================================
;;Takes a cluster number in ax and returns the next-cluster value found at that
;;location in the FAT in ax
;;This could be made faster by treating the sector load location as a cache
;;and only re-reading it if the cached sector number does not match the request
get_next_cluster:

    push dx
    push di

    ;;Calculate the FAT sector in which our cluster resides
    push ax       ;back up the original cluster number
    div 0x100     ;divide the cluster by the number of cluster entries per sec
    push dx       ;back up the remainder value
    add ax, [V_RESSECT]  ;Offset that value by the number of reserved sectors
    call lba_to_csh  ;Convert block address to CSH
    xor es, es
    mov dx, 0x500        ;Read into 0x0:0x500
    call read_sector

    ;;Now that the sector is read, get the entry within the sector
    pop bx       ;restore the remainder of the division
    xor di, di
    mov ax, [bx+di+0x500] ;read the value at the offset

    push di
    pop dx
    ret
;===============================================================================


;;Static data is stashed here

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
