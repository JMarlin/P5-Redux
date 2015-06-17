[org 0x7e00]
[bits 16]

jmp main

loaded db 'Jumped to second stage code', 0
yes db 'P5KERN.BIN found!', 0
no db 'P5KERN.BIN was not found on this disk.', 0
entrycount dd 0             ;the counter of entries in the root directory that aren't P5KERN.BIN.
datapointer dd 0x500        ;the pointer for reading disk data starting at a base of 0x500, where the root directory data will be read to.
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

main:
mov edx, loaded
call printstr

hang:
jmp hang

times 3301 db 0
