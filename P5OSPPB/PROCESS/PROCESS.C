#include "../core/kernel.h"
#include "process.h"
#include "../ascii_io/ascii_o.h"
#include "../ascii_io/ascii_i.h"
#include "../core/syscall.h"


unsigned char fake[6];
unsigned char* insPtr;
context V86Context;
unsigned short V86RetCS;
unsigned int V86RetIP;
int intVect = 0; 

//We'll ACTUALLY use this in the future
context* activeContext = &V86Context;


void returnToProcess(context* newContext) {
            
    //Restore the running context
    old_esp = newContext->esp;
    old_cr3 = newContext->cr3;
    old_eip = newContext->eip;
    old_eflags = newContext->eflags;
    old_eax = newContext->eax;
    old_ecx = newContext->ecx;
    old_edx = newContext->edx;
    old_ebx = newContext->ebx;
    old_ebp = newContext->ebp;
    old_esi = newContext->esi;
    old_edi = newContext->edi;
    old_es = newContext->es;
    old_cs = newContext->cs;
    old_ss = newContext->ss;
    old_ds = newContext->ds;
    old_fs = newContext->fs;
    old_gs = newContext->gs;
    old_err = newContext->err;
    
    returnToProc();
}


void kernelDebug(void) {

    //Kernel debug, should be moved to its own function
    prints("INTERRUPT HAS RETURNED CONTROL TO THE KERNEL\n");
    prints("Previous State:\n");
    prints("eax: 0x"); printHexDword(old_eax); prints("  ebx: 0x"); printHexDword(old_ebx); prints("\n");
    prints("ecx: 0x"); printHexDword(old_ecx); prints("  edx: 0x"); printHexDword(old_edx); prints("\n");
    prints("esp: 0x"); printHexDword(old_esp); prints("  ebp: 0x"); printHexDword(old_ebp); prints("\n");
    prints("esi: 0x"); printHexDword(old_esi); prints("  edi: 0x"); printHexDword(old_edi); prints("\n");
    prints("cr3: 0x"); printHexDword(old_cr3); prints("  eip: 0x"); printHexDword(old_eip); prints("\n");
    prints("eflags: 0x"); printHexDword(old_eflags); prints("\n");
    prints("es: 0x"); printHexWord(old_es); prints("  cs: 0x"); printHexWord(old_cs); prints("\n");
    prints("ss: 0x"); printHexWord(old_ss); prints("  ds: 0x"); printHexWord(old_ds); prints("\n");
    prints("fs: 0x"); printHexWord(old_fs); prints("  gs: 0x"); printHexWord(old_gs); prints("\n");
    
    //Get the error code
    prints("Error code: 0x"); printHexDword(old_err); prints("\n");
    
    //Get the command byte that the processor failed on:
    prints("Failed instructions: 0x"); printHexByte(insPtr[0]);
    prints(", 0x"); printHexByte(insPtr[1]); 
    prints(", 0x"); printHexByte(insPtr[2]);
    prints(", 0x"); printHexByte(insPtr[3]);
    prints(", 0x"); printHexByte(insPtr[4]);
    prints("\n");
}


//This needs to be moved to its own module eventually
void V86Entry(void) {
    
    unsigned short seg, off;
  
    switch(insPtr[0]) {
    
        //INT X
        case 0xCD:
            //If it was a service call, forward it to the syscall handler    
            if(insPtr[1] == 0xFF) {
            
                syscall_number = activeContext->eax & 0xFFFF;
                syscall_param1 = activeContext->ebx & 0xFFFF;
                syscall_param2 = activeContext->ecx & 0xFFFF;
                syscall_exec();
            } else {

                ((unsigned char*)0x81818)[0] = 12;                
                intVect = 0x00000000 + (insPtr[1] & 0xFF);
                intVect *= 4;
                off = ((unsigned short*)intVect)[0];
                seg = ((unsigned short*)intVect)[1];;
                prints("V86 Interrupt #"); printHexByte(insPtr[1]);
                prints(" -> "); printHexWord(seg);
                prints(":"); printHexWord(off);
                prints("\n");
                
                //Hopefully we don't get nested INTs, and in that case we should really make a stack of these
                V86RetCS = activeContext->cs;
                V86RetIP = activeContext->eip + 2;
                activeContext->cs = seg;
                activeContext->eip = (((unsigned int)off) & 0xFFFF);
            }
            returnToProcess(activeContext);
            break;
        
        //IRET
        case 0xCF:
            activeContext->cs = V86RetCS;
            activeContext->eip = V86RetIP;
            returnToProcess(activeContext);
            break;
        
        //INT 3 (debug) or anything else
        case 0xCC:
        default:
            kernelDebug();
            scans(5, fake);
            activeContext->eip++;
            returnToProcess(activeContext);
            break;       
    }
}


void kernelEntry(void) {

    //Backup the running context
    activeContext->esp = old_esp;
    activeContext->cr3 = old_cr3;
    activeContext->eip = old_eip;
    activeContext->eflags = old_eflags;
    activeContext->eax = old_eax;
    activeContext->ecx = old_ecx;
    activeContext->edx = old_edx;
    activeContext->ebx = old_ebx;
    activeContext->ebp = old_ebp;
    activeContext->esi = old_esi;
    activeContext->edi = old_edi;
    activeContext->es = old_es;
    activeContext->cs = old_cs;
    activeContext->ss = old_ss;
    activeContext->ds = old_ds;
    activeContext->fs = old_fs;
    activeContext->gs = old_gs;
    activeContext->err = old_err;

    //Convert the V86 seg:off into a linear address
    //This is V86 specific and should be moved
    insPtr = (char*)(((((unsigned int)old_cs)&0xFFFF) << 4) + (((unsigned int)old_eip) &0xFFFF));

    //Switch to the V86 monitor if the thread was a V86 thread
    if(old_eflags & 0x20000) {
    
        V86Entry();
    } else {
    
        //Otherwise, for now we just dump the system state and move on
        kernelDebug();
        scans(5, fake);
        returnToProcess(activeContext);
    }
}


void terminateProcess(proc* p) {

    //This is just some bullshit 'go back to console' placeholder
    sys_console();    
}
