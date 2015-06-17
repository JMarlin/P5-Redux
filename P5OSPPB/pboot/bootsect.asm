[ORG 0x7c00]
[BITS 16]

jmp boot
db 0

;Most of the data below is calculated by the image generation script and then
;passed in as assembler macros.
;================= FAT12 data ===================
db 'MYFILE  '            ;OEM name
dw SECTORSZ              ;Bytes per sector
db SECTPERCLUSTER        ;Sectors per cluster
dw RESSECT+1             ;Number of reserved sectors
db FATCOPIES             ;Copies of the FAT
dw ROOTENTRIES           ;Number of clusters in the root directory entry
dw TOTALSECT             ;Number of total sectors on disk
db 0xF0                  ;Type of disk (F0h = 1.44m floppy, F8h = hard disk)
dw SECTPERFAT            ;sectors per FAT (192 is large, but allows for 32-meg worth of clusters)
;================= FAT12 data ===================

tststr db 'Bootsector Loaded.\n', 0
failstr db 'FAIL', 0
bdrive db 0

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

boot:

mov [bdrive], dl ;store the drive number

mov edx, tststr
call printstr

mov ax, 0
mov es, ax

mov ah, 0x0
int 0x13
mov ah, 0x02
mov al, 2
xor ch, ch
mov cl, 2
mov bx, 0x7e00
xor dh, dh
mov dl, [bdrive]
int 0x13
jc ufukd ;disk read failed

jmp 0x7e00

ufukd:
mov edx, failstr
call printstr
hurr:
jmp hurr

times 510-($-$$) db 0x0
dw 0xAA55             ;Boot signature
