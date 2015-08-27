;________________________________________________________________________
;|**********************************************************************|
;|**   File: P5kern.asm      ***     Original Author: Joseph Marlin   **|
;|**********************************************************************|
;|**   Administrator Contact: stithyoshi@users.sourceforge.net        **|
;|**********************************************************************|
;------------------------------------------------------------------------


;This file was loaded to memory sector 0 offset 0x500 by the boot sector
;after which the boot sector jumped to 0x500. Therefore, our code comtinues
;here.

;*****************************************************
;**              NASM Compile Directives            **
;*****************************************************

[ORG 0x1700]                                      ;Tell NASM that our program originates at memory location 0x500 to make all of 
                                                 ;our jmps correct
[BITS 16]                                        ;Assemble the following code as 16-bit 8086 code 

;*****************************************************
;**                Beginning Of Code                **
;*****************************************************

jmp main                                         ;jmp to the starting point of our kernel.

;*****************************************************
;**          Data To Be Referenced Later            **
;*****************************************************

version_name db 'vX - "Pink-CI" (pink-ki)',0     ;The kernel itself uses none of this, this is printed by the bootloader.
prompt db 10,13,10,13,'P5-> ',0, '                                                  ';This is the string we will use as our prompt.
                                                 ;There is so much extra space after the 0 of the string because this could possibly
                                                 ;be changed to a string up to 50 characters in length.
badc db 10,13,' Error: PEBKAC',0                 ;This is our humorous error message.
inbuf db '                                                  ', 0 ;This will be our 50 character input buffer.
oset dd 0                                        ;This is used by the storestring function to temporarily store the value of ebx.
ok db 10,13,' OK',0                              ;Our happy sucess message..
tank db 0                                        ;This will temporarily store the character being read from the keyboard.
com1 db 'CLR',0                                  ;This is the string value of the CLR command which will be used for comparison.
com2 db 'CHPROMPT', 0                            ;The string value for the CHPROMPT command.
bgmsg db 10,13,'     -> ',0                      ;This is the prompt CHPROMPT displays as a prompt for the new prompt string.

pchar:                                           ;This is the exact same pchar function from our boot sector.
mov ah, 0x0E                                     ;Function 0xE, print a character to the screen.
mov bl, 0                                        ;bl = 0, no special characteristics.
int 0x10                                         ;Int 0x10, print the character.
ret                                              ;Return.

newline:                                         ;This is a rather pointless function to write a newline.
mov al, 10                                       ;Character to move down one line.
call pchar                                       ;Print.
mov al, 13                                       ;Character to return to the beginning of the line.
call pchar                                       ;Print.
ret                                              ;Return.

printstr:                                        ;Printstr: print a string pointed to by edx.
printstart:                                      ;Top of loop.
mov bh, 0x0F                                     ;Set color to white.
mov al, [edx]                                    ;move the value pointed to by edx into al.
call pchar                                       ;Print the character at edx.
add edx, 1                                       ;Increase edx.
cmp byte [edx], 0                                ;Have we reached the end of the string yet?
je printend                                      ;Yes, get out of here.
jmp printstart                                   ;No, go back to the top.
printend:                                        ;This is the end!
ret                                              ;Return.
 
clear:                                           ;Clear: This will be called as the response to the CLR command.
mov cx, 2040                                     ;We will first print enough null characters to the screen to fill it. this is a 80*25 
                                                 ;screen, so we print 1640 null characters plus a couple of lines just for good measure. 
                                                 ;We use cx as our countdown.
tippy:                                           ;Top of loop.
mov al, 0                                        ;Make al 0, a null character.
call pchar                                       ;Print a null character to the screen.
dec cx                                           ;decrease cx.
jz verybot                                       ;If cx has finally reached 0 and we have therefore printed all 2040 null characters, go 
                                                 ;to the next bit.
jmp tippy                                        ;Otherwise, go back to the top.
verybot:                           
mov cx, 0                                        ;I wasn't quite sure what registers controlled the position of the cursor for the move 
mov dx, 0                                        ;cursor function, so I just cleared them all.
mov bx, 0
mov ax, 0
mov ah, 2                                        ;Then I put 2 into ah for the cursor moving function of int 0x10. 
int 0x10                                         ;And then call the interrupt!
ret                                              ;Now the screen is all clear and the cursor is at the top of it, so we'll return.

strcmp:                                          ;Strcmp: self-explanatory string comparison function which takes edx and ecx as pointers to 
                                                 ;the strings to be compared.
mov al, [edx]                                    ;Move the data at edx into al.
mov bl, [ecx]                                    ;And the data at ecx into bl.
cmp al, bl                                       ;Then compare the two values.
jne fail                                         ;If they're not the same then we know that the strings cannot possibly be the same, 
                                                 ;so we Jump if Not Equal to fail. 
inc edx                                          ;Now we'll move to the next character in each string.
inc ecx
mov al, [edx]                                    ;And we'll move the new characters back into al and bl.
mov bl, [ecx]          
cmp al, 0                                        ;Have we reached the end of one of the strings?
je alzer                                         ;Then jump to the next section of testing.
jmp strcmp                                       ;Otherwise, keep checking until we get a difference in characters.
alzer:                              
cmp bl, 0                                        ;Is bl 0 too?
jne fail                                         ;If it's not then they're not the same string. Go to fail. Do not pass Go. Do not collect $200.
jmp gedd                                         ;Otherwise they are the same string and we can go to gedd.
fail:                                           
mov al, 0                                        ;al is our return value, 0 for fail and 1 for pass.
jmp gbdd                                         ;now that it's set, we go to the return.
gedd:                      
mov al, 1                                        ;It passed, so we eturn a 1.
gbdd:        
ret                                              ;And return.

parser:                                          ;This function will read our input in inbuf and react accordingly.
mov edx, inbuf                                   ;Strcmp arg1, inbuf.
mov ecx, com1                                    ;Strcmp arg2, com1 (CLR).
call strcmp                                      ;Compare the inbuf with our first possible command.
cmp al, 1                                        ;Was it sucessful?
je come1                                         ;Yes, move on.
jmp comt2                                        ;No, check the next possible command.
come1:        
mov edx, ok                                      ;Printstr arg1, ok.
call printstr                                    ;Print our ok mesage to the screen.
call clear                                       ;Clear the screen.
jmp eloo                                         ;We're done.
comt2:                                    
mov edx, inbuf                                   ;Strcmp arg1, inbuf.
mov ecx, com2                                    ;Strcmp arg2, com2.
call strcmp                                      ;Compare inbuf with the second possible command.
cmp al, 1                                        ;Sucessful?
je come2                                         ;Yup, move on.
jmp crap                                         ;Nope, it's neither of teh commands. Fail.
come2:            
call bill                                        ;Get input and cange the prompt accordingly.
mov edx, ok                                      ;Printstr arg1, ok
call printstr                                    ;Print out ok message.
jmp eloo                                         ;exit with sucess.
crap: 
mov edx, badc                                    ;Point edx to our error message. Oops! PEBKAC!
call printstr                                    ;Print error message.
eloo:                  
ret                                              ;get out of here!

bill:                                            ;Bill, an approprietly named function to complete CHangePROMPT.
mov edx, bgmsg                                   ;Point to our prompt.
call printstr                                    ;Print the prompt.
call storestring                                 ;Get a string of input and put it in inbuf.
mov edx, inbuf                                   ;Now we'll copy the input string 
mov ecx, prompt+4                                ;To where the prompt is stored, after the carriage return data.
call strcop                                      ;Do it.
ret                                              ;Go back.

strcop:                                          ;Strcop, copy a string from one location to another.
mov al, [edx]                                    ;Mov the data pointed at by edx into al.
mov bl, [ecx]                                    ;Mov the data pointed at by ecx into bl.
cmp al, 0                                        ;Is al 0?
je botendit                                      ;Yup, we've reached the end of the string and we can therefore end.
mov [ecx], al                                    ;Put the data in al to the memory location in ecx. Effectively we have copied [edx] into [ecx].
inc edx                                          ;Now we'll point at the next characters.    
inc ecx
jmp strcop                                       ;and go back to the top.
botendit:             
mov byte [ecx], 0x20                             ;We'll add a space to the string we're copying.
inc ecx                                          ;We'll move to the next character...
mov byte [ecx], 0                                ;Tack on a null...
ret                                              ;And exit happily.

storestring:                                     ;Here we'll get input from the keyboard and place it into inbuf.
mov cx, 50                                       ;We're taking in 50 chars, count down with cx.
mov ebx, inbuf                                   ;Put the address of inbuf into ebx.
mov [oset], ebx                                  ;And store it into our oset memory location.
topper:                                          ;Top of the loop.
mov ah, 0                                        ;Int 0x16 function 0, read ASCIIcharacter from the keyboard.
int 0x16                                         ;Read it.
mov [tank], al                                   ;The character is returned in al. We put that into a storage [tank].
mov al, [tank]                                   ;Let's get our stored character for a moment again...
cmp al, 13                                       ;And see if it was a RETURN (ASCII code 13 = carriage return)
je enderit                                       ;If it is, then they're done and we can finish up.
cmp al, 8                                        ;Otherwise, we should check to see if they pressed backspace.
je bksp                                          ;If so, we'll handle that.
cmp cx, 0                                        ;Have we hit our character limit?
je bottom                                        ;Then we'll jump past all of the printing and storing stuff because we can't anymore.
mov al, [tank]                                   ;Now that we've handled the possible excptions, we'll start storage by pulling the 
                                                 ;character from tank again.
mov ebx, [oset]                                  ;and move the pointer to inbuf to ebx.
mov [ebx], al                                    ;And move the character in al to the character to the character currently pointed to by ebx.
mov al, 0                                        ;Clear al.
mov al, [ebx]                                    ;Use the current inbuf character as the argument to pchar so we can see what we typed.
mov [oset], ebx                                  ;Put the current inbuf pointer into oset again.
call pchar                                       ;print that character.
mov ebx, [oset]                                  ;Put the inbuf pointer into ebx AGAIN.
add ebx, 1                                       ;Then increase it to point to the next char.
mov [oset], ebx                                  ;And store it.
sub cx, 1                                        ;We've added another character to the buffer, so we have to take that out of our number of 
                                                 ;remaining spaces counter. 
bottom:           
jmp topper                                       ;We were full up, so let's go back to the top and get characters until there's a backspace or 
                                                 ;a return.
bksp:                                            ;Handle a backspace.
call pchar                                       ;Print whatever's in al (ASCII 8, a backspace).
mov al, 0                                        ;Then print a null.
call pchar                                       ;Do it.
mov al, 8                                        ;Then print another backspace.
call pchar                                       ;Do it.
mov ebx, [oset]                                  ;Move out pointer back into ebx,
sub ebx, 1                                       ;decrement it,
mov [oset], ebx                                  ;and put it back.
add cx, 1                                        ;We need to make sure space is properly allocated.
jmp topper                                       ;We've moved the cursor back and made sure we're pointing at the right thing. Go back to the top.
enderit:                                         ;This is what happens when we hit return...
mov byte [ebx], 0                                ;Tack a 0 onto the input string.
ret                                              ;And just haul ass out of there.

;*****************************************************
;**             Our Main Section Of Code            **
;*****************************************************

main:                                            ;The actual main loop is pretty simple.
mov edx, prompt                                  ;Point edx to the prompt message.
call printstr                                    ;And print it.
call storestring                                 ;Get keyboard input and store it in inbuf.
call parser                                      ;Then react accordingly to it.
jmp main                                         ;And do that forever. 
