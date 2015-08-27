[ORG 0x7c00]
[BITS 16]

;Each FAT is 9 sectors long
;The root directory is 14 sectors long
;fat entries are stored two to three bits in reverse order and big-endian storage.
;Therefore a series of fat entries 0xff0 0xfff 0x003 0x00f would be stored as:
;  0xf0ffff03f0ff

jmp boot

;FAT12 data

db 'MYFILE  '            ;OEM name
db 0x00, 0x02            ;Bytes per sector
db 1                     ;Sectors per cluster
db 0x03, 0x00            ;Number of reserved sectors
db 2                     ;Copies of the FAT
db 0xE0, 0x00            ;Number of root directory entries
db 0x40, 0x0b            ;Number of total sectors on disk
db 0xF0                  ;Type of disk (F0h = 1.44m floppy, F8h = hard disk)
db 0x09, 0x00            ;sectors per FAT
db 0x12, 0x00            ;SPT
db 0x02, 0x00            ;Number of heads
dw 0                     ;Number of hidden sectors
db 0xE0, 0x00, 0x00, 0x00;total sectors 2

;End of FAT12 data

yes db 'P5KERN.BIN found!', 0
no db 'P5KERN.BIN was not found on this disk.', 0
tststr db 'BS Loaded', 0
failstr db 'FAIL', 0
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
jc ufukd
mov cl, [activesector]
add cl, 1
mov [activesector], cl
ret

printstra:
printstarta:
mov bh, 0x0F
mov al, [edx]
mov ah, 0x0E
mov bl, 0
int 0x10
add edx, 1
cmp byte [edx], 0
je printenda
jmp printstarta
printenda:
ret

boot:

mov [bdrive], dl

mov edx, tststr
call printstra

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
call printstra
hurr:
jmp hurr

times 510-($-$$) db 0x0
dw 0xAA55             ;Boot signature

;reserved sectors 2 and 3

jmp main

printstr:
printstart:
mov bh, 0x0F
mov al, [edx]
mov ah, 0x0E
mov bl, 0
int 0x10
add edx, 1
cmp byte [edx], 0
je printend
jmp printstart
printend:
ret

temp dw 0

clustertocsh:
  mov edx, 0
  add ax, 34

  ;mov edx, gbd
  ;call printstr

  div word [spt]

  ;mov edx, gpd
  ;call printstr

  mov [tracknum], ax
  inc dx
  mov [tsector], dl
  mov dl, [tsector]
  sub dl, 2
  mov [tsector], dl
  mov edx, 0
  mov ax, [tracknum]
  div word [headnum]
  mov [ttrack], al
  mov [thead], dl
  mov cl, [tsector]
  inc cl
  mov [tsector], cl
  mov dh, [thead]
  mov cl, [tsector]
  mov ch, [ttrack]
  ret

  gbd db 10,13,' got to just before first divide',0
  gpd db 10,13,' passed first divide',0

cmpname:
mov edx, [datapointer]
cmp byte [edx], 'P'
je l5
jmp cmpfail
l5:
cmp byte [edx+1], '5'
je lK
jmp cmpfail
lK:
cmp byte [edx+2], 'K'
je lE
jmp cmpfail
lE:
cmp byte [edx+3], 'E'
je lR
jmp cmpfail
lR:
cmp byte [edx+4], 'R'
je lN
jmp cmpfail
lN:
cmp byte [edx+5], 'N'
je lS1
jmp cmpfail
lS1:
cmp byte [edx+6], ' '
je lS2
jmp cmpfail
lS2:
cmp byte [edx+7], ' '
je lB
jmp cmpfail
lB:
cmp byte [edx+8], 'B'
je lI
jmp cmpfail
lI:
cmp byte [edx+9], 'I'
je lN2
jmp cmpfail
lN2:
cmp byte [edx+10], 'N'
je cmpsuccess
cmpfail:
mov al, 0
ret
cmpsuccess:
mov al, 1
ret

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
clusterone:
mov ax, [ce1]
mov [cluster], ax
jmp toout
clustertwo:
mov ax, [ce2]
mov [cluster], ax
toout:
ret

prclusternum:
mov ax, [cluster]
mov [dig1], ax
mov [dig2], ax
mov [dig3], ax
mov [dig4], ax
mov ax, [dig1]
and ax, 0xf000
shr ax, 12
add ax, 48
mov [dig1], ax
mov ax, [dig2]
and ax, 0x0f00
shr ax, 8
add ax, 48
mov [dig2], ax
mov ax, [dig3]
and ax, 0x00f0
shr ax, 4
add ax, 48
mov [dig3], ax
mov ax, [dig4]
and ax, 0x000f
add ax, 48
mov [dig4], ax
mov al, 10
call pchar
mov al, 13
call pchar
mov ax, [dig1]
call pchar
mov ax, [dig2]
call pchar
mov ax, [dig3]
call pchar
mov ax, [dig4]
call pchar
mov ah, 1
int 0x21
ret

dig1 dw 0
dig2 dw 0
dig3 dw 0
dig4 dw 0

main:

call readsec

top:
call cmpname
cmp al, 1
je mainsuccess
mov edx, [datapointer]
add edx, 32
mov [datapointer], edx
mov edx, [entrycount]
add edx, 1
mov [entrycount], edx
mov edx, [entrycount]
cmp edx, 244
je mainfail
mov edx, [datapointer]
cmp edx, 0x700
je nextsector
jmp top
nextsector:
mov dword [datapointer], 0x500
call readsec
jmp top
mainfail:
mov edx, no
call printstr
jmp hold
mainsuccess:
mov edx, yes
call printstr
mov edx, [datapointer]
add edx, 0x1a
mov [datapointer], edx
mov al, [edx]
mov edx, [datapointer]
add edx, 1
mov [datapointer], edx
mov ah, [edx]
mov [cluster], ax


mov ah, 0x0
int 0x13
mov ah, 0x02
mov al, 9
xor ch, ch
mov cl, 4
mov bx, 0x500
xor dh, dh
xor dl, dl
int 0x13

nextcluster:
mov ax, [cluster]
call clustertocsh
mov bx, [kernelsecoffset]
add bx, 0x1700
mov ah, 0x02
mov al, 1
mov dl, 0
int 0x13
mov bx, [kernelsecoffset]
add bx, 512
mov [kernelsecoffset], bx
mov ax, [cluster]
mov [clusternum], ax
call  get_cluster_fat_entry
mov ax, [cluster]
mov bx, 0x0fff
grap:
cmp ax, bx
je done
dec bx
cmp bx, 0x0ff7
je uuber
jmp grap
uuber:
jmp nextcluster

hold jmp hold

done:
mov edx, vertop
call printstr
mov edx, 0x170C
call printstr
mov edx, verbot
call printstr

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

times (512*3)-($-$$) db 0

;fat1

db 0xf0, 0xff, 0xff, 0xff, 0x0f, 0x00

times (512*9)-6 db 0

;fat2

db 0xf0, 0xff, 0xff, 0xff, 0x0f, 0x00

times (512*9)-6 db 0

;root dir

db 'PBOOT R1', '   '
db 0x08
db 0
db 0
dw 0
dw 0
dw 0
dw 0
dw 0
dw 0
dw 0
dd 0

db 'SUCCESS ', 'TXT'
db 0x20
db 0
db 0
dw 0
dw 0
dw 0
dw 0
dw 0
dw 0
db 0x02, 0x00
db 0x0E, 0x01, 0x00, 0x00

times (512*14)-64 db 0

;sector 2-2880

db 'If you are reading this file it means that this floppy disk has been successfully set up as a PBOOT floppy. Place the kernel into the root directory of this floppy with the name P5KERN.BIN and boot from this floppy to run your kernel code.'

times (512*2880)-($-$$) db 0
