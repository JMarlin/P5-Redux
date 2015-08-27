;________________________________________________________________________
;|**********************************************************************|
;|**   File: P5Boot.asm      ***     Original Author: Joseph Marlin   **|
;|**********************************************************************|
;|**   Administrator Contact: stithyoshi@users.sourceforge.net        **|
;|**********************************************************************|
;------------------------------------------------------------------------


;This is the boot sector for P5 version "Pink-CI" (pronounced "Pink-kie")
;The code we're writing will be compiled to make binary data for the first 512 byte
;sector of a floppy disk. This sector is called the boot sector and is loaded 
;by the BIOS after it's done setting up the computer into memory location 0x7c00.
;The BIOS then starts the processor executing at 0x7c00 so that we can use that code
;to start up an operating system (or something like that, it can be code to do anything).


;*****************************************************
;**              NASM Compile Directives            **
;*****************************************************

[ORG 0x7c00]                                     ;Tell NASM that our program originates at memory location 0x7c00 to make all of 
                                                 ;our jmps correct
[BITS 16]                                        ;Assemble the following code as 16-bit 8086 code 

;*****************************************************
;**                Beginning Of Code                **
;*****************************************************

jmp main                                         ;unconditional jump over the following data (which the processor would interpret 
                                                 ;as gibberish if we didn't jmp over it) to the label 'main'

;*****************************************************
;**          Data To Be Referenced Later            **
;*****************************************************

; The only two chunks of data we need are both part 
;of the boot message we want this program to print.
   
; The first part of the data declarations is a label 
;which represents the location in memory the first 
;byte of the following data or code resides at so
;that when we jmp we won't have to figure out the 
;address ourselves, which is a pain in the arse.
; The second part is how big the chunks of data are 
;that we're defining. In this case, we use 'db' to
;denote that we are defining byte (8-bit) chunks of 
;data. The other options are dw (a word, 16 bits)
;and dd (a double word, 32 bits).
; The third part is the data that we are actually 
;defining. In the case of bytes, we can put 8-bit 
;ASCII data right togeather on the same line by 
;putting the data in a string surrounded by quotation
;marks. With bytes and every other size of data we 
;can do the same thing with binary (suffix 'b' ie: 
;01001011b), hexadecimal (prefix '0x' or suffix 'h')
;and regular decimal numbers by simply putting commas 
;between data elements. The two types of definition 
;for bytes can be mixed and matched.      

; The byte of 0 at the end of the string is 
;interpreted as the end of the string by the 
;string printing function we define later. 
;10 is the ASCII value which moves the cursor
;down one line and 13 moves the cursor to the 
;beginning of the line it's on.

boot_msg_h db 'Protical5 ',0                     ;The 'High' part of the message we want our bootloader to print.
boot_msg_e db ' found.',10,13,'Booting...',0     ;The End part. When we print our message we'll stick a string denoting
                                                 ;the kernel version that we'll pull from the second disk sector between 
                                                 ;the high and the end.  
                                                 
;*****************************************************
;**          Functions To Be Called Later           **
;*****************************************************

; The plain text at the top of each function is 
;just another way of writing labels. 

pchar:                                           ;Function pchar: Print a character to video memory
mov ah, 0x0E                                     ;Firstly we move the number 0xE into the high 8-bits of the A register;
                                                 ;this tells the BIOS that we're going to print a character to the screen 
                                                 ;when interrupt 0x10 is called.
mov bl, 0                                        ;We then move a 0 into the low 8-bits of the B register, telling the interrupt 
                                                 ;code we'll call that there are no special features we want added to our text.
int 0x10                                         ;Finally we interrupt the processor and pass the processor the interrupt number 
                                                 ;(or 'interrupt vector') 0x10 which is the BIOS interrupt handler for video services.  
                                                 ;We use interrupts defined at boot time by the BIOS for now that will use some trickier 
                                                 ;code that requires knowledge about chips connected to the processor that we really
                                                 ;don't need to do ourselves right now. It's a convinient and common shortcut.
ret                                              ;And we use the ret command to jump back to the point at which this function was 
                                                 ;called.

resetdrive:                                      ;Function resetdrive: Reset floppy drive A
xor ah, ah                                       ;Zero out ah by xoring it with itself. This is BIOS interrupt 0x13 function 0,
                                                 ;reset drive
xor dl, dl                                       ;Zero out dl to identify which floppy drive to reset (0, the first drive, DOS drive 
                                                 ;A).
int 0x13                                         ;And then call the BIOS definied interrupt 0x13: disk services.
ret                                              ;return.

loadme:                                          ;Function loadme: Load the second sector of the floppy disk (our kernel) into memory
mov ah, 0x02                                     ;Int 0x13, function 2: read sector(s)
mov al, 2                                        ;Ammount of sectors to read. Our kernel spans a little more than one. 
mov cx, 0                                        ;Move 0 into ch telling int 0x13 that our sector is on track 0 of the disk  
mov cl, 2                                        ;Move 2 into cl telling int 0x13 to start reading at sector 2
mov dh, 0                                        ;Move 0 into dh telling int 0x13 to read from disk 0
                                                 ;There is yet another peice of data that we have to give to int 0x13 for it to work 
                                                 ;right and that is the location in memory we want to put the floppy data. This is
                                                 ;defined through es (the segment register which sets which 64kb memory sector we're in)
                                                 ;and bx (which will hold the actual offset in the current sector that we want to put the
                                                 ;data at). By not messing with these in the function itself we can set them right before
                                                 ;calling the function, which is how you pass arguments to a function.
int 0x13                                         ;As usual, call the interrupt
ret                                              ;and return.

;*****************************************************
;**             Our Main Section Of Code            **
;*****************************************************

main:                                            ;The label main: This is where we jmp to in the first command in this code. 

mov edx, 0                                       ;Put 0 into the 32-bit D register.

printstart:                                      ;We'll use this label and a jmp to create a while loop.
mov bh, 0x0F                                     ;bh is an argument to the pchar function that defines the character color
                                                 ;NOTE: the color itself is ignored by most BIOSes.
mov al, [edx+boot_msg_h]                         ;[x] Represents a movement of the data at the memory address stored in x
                                                 ;or movement of data to the same address. In this case, we are adding the value 
                                                 ;in edx to the address of boot_msg_h so we can chang it to reference all of the 
                                                 ;data in the string we defined with a loop that changes edx. 
call pchar                                       ;al was the argument to int 0x10 that defined the character to print. Therefore 
                                                 ;we have effectively moved the edx-th character in the string to the screen. 
add edx, 1                                       ;increase edx by one. (edx++ or edx = edx + 1)
cmp byte [edx+boot_msg_h], 0                     ;check a byte of data at edx+boot_msg_h (ie: the next character in the string)
                                                 ;to see what it is relative to 0.
je printend                                      ;Jump if Equal to the label printend
jmp printstart                                   ;If we haven't jumped by now (ie: if we didn't detect a 0 in the next character in 
                                                 ;the string) we'll jump back up to printstart and do the whole thing over again until 
                                                 ;we find a 0.
printend:                                        ;We jump here to get out of our loop.

call resetdrive                                  ;Reset the floppy drive
mov ax, 0                                        ;we want to set es and bx with our memory sector and offset data for our loadme 
                                                 ;function, but es is a specal register that we cant put data straight into so we first 
                                                 ;set ax to our desired memory sector (0) 
mov es, ax                                       ;and then put the data in ax into es.
mov bx, 0x500                                    ;0x500 is where we'll put the kernel data. This happens to be the place in the memory where 
                                                 ;the area in the 1mb of memory we have acess to in the 8086 compatibility mode the processor
                                                 ;starts up in that isn't reserved by the BIOS or the video card starts. I could have put the 
                                                 ;kernel anywhere in that area, but the very beginning of our usable memory seemed nice to me. 
call loadme                                      ;And then we just run our function and load the data.

mov edx, 0                                       ;Now we're going to go into the same kind of loop we did before to print the first part
                                                 ;of our boot message, so we set edx to 0 again.
mintstart:                                       ;Top of the loop.
mov bh, 0x0F                                     ;Color (white)
mov al, [edx+0x503]                              ;Now instead of ofsetting from a label in our own code, we'll offset from 0x503, which is 
                                                 ;where the beginning of the version string in the kernel code will end up after we read it to
                                                 ;memory.
call pchar                                       ;Print the character.
add edx, 1                                       ;increase edx.
cmp byte [edx+0x503], 0                          ;Is the next byte 0?
je mintend                                       ;Yes, skip to mintend
jmp mintstart                                    ;No, go back to mintstart (I have very descriptive labels, no?)
mintend:                                         ;Outside the loop.

mov edx, 0                                       ;Same loop AGAIN... this time to print the rest of our message.
pintstart:                                       ;Top
mov bh, 0x0F                                     ;Color
mov al, [edx+boot_msg_e]                         ;Character from label offset
call pchar                                       ;Print it!
add edx, 1                                       ;edx++
cmp byte [edx+boot_msg_e], 0                     ;0?
je pintend                                       ;Yup, exit.
jmp pintstart                                    ;No! Go to top!
pintend:                                         ;Free at last!
jmp 0x500                                        ;jmp to the kernel code that was stored at memory offset 0x500
here jmp here                                    ;This isn't really needed as the processor won't ever get to it, but this hangs the 
                                                 ;computer. I'm sure you can tell how.
times 510-($-$$) db 0                            ;This is a NASM dirsective to put bytes of 0 into the file until we're reached the 510th byte  
                                                 ;in the file.
dw 0xAA55                                        ;Now we put a word of 0xAA55 at the end of the boot sector (bytes 511 and 512). When the BIOS 
                                                 ;loads this sector to the memory for booting, it first checks these two bytes to make sure that 
                                                 ;the disk is, in fact, supposed to be bootable.
