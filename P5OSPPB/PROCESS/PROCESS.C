#include "../core/kernel.h"
#include "process.h"
#include "../ascii_io/ascii_o.h"
#include "../ascii_io/ascii_i.h"
#include "../core/syscall.h"
#include "../core/util.h"
#include "../core/expt.h"
#include "../fs/fs.h"


unsigned char fake[6];
unsigned char* insPtr;
context V86Context, usrContext;
int intVect = 0;
process* procTable = (process*)0x2029A0;
int nextProc = 0;

//We'll ACTUALLY use this in the future
process* p = (process*)0;


void returnToProcess(process* newProcess) {

    p = newProcess();

    //Restore the running context
    old_esp = newProcess->ctx.esp;
    old_cr3 = newProcess->ctx.cr3;
    old_eip = newProcess->ctx.eip;
    old_eflags = newProcess->ctx.eflags;
    old_eax = newProcess->ctx.eax;
    old_ecx = newProcess->ctx.ecx;
    old_edx = newProcess->ctx.edx;
    old_ebx = newProcess->ctx.ebx;
    old_ebp = newProcess->ctx.ebp;
    old_esi = newProcess->ctx.esi;
    old_edi = newProcess->ctx.edi;
    old_es = newProcess->ctx.es;
    old_cs = newProcess->ctx.cs;
    old_ss = newProcess->ctx.ss;
    old_ds = newProcess->ctx.ds;
    old_fs = newProcess->ctx.fs;
    old_gs = newProcess->ctx.gs;
    old_err = newProcess->ctx.err;

    kernelDebug();
    
    returnToProc();
}


void kernelDebug(void) {

    //Kernel debug, should be moved to its own function
    prints("INTERRUPT HAS RETURNED CONTROL TO THE KERNEL\n");
    prints("Previous State:\n");
    prints("eax: 0x"); printHexDword(p->ctx.eax); prints("  ebx: 0x"); printHexDword(p->ctx.ebx); prints("\n");
    prints("ecx: 0x"); printHexDword(p->ctx.ecx); prints("  edx: 0x"); printHexDword(p->ctx.edx); prints("\n");
    prints("esp: 0x"); printHexDword(p->ctx.esp); prints("  ebp: 0x"); printHexDword(p->ctx.ebp); prints("\n");
    prints("esi: 0x"); printHexDword(p->ctx.esi); prints("  edi: 0x"); printHexDword(p->ctx.edi); prints("\n");
    prints("cr3: 0x"); printHexDword(p->ctx.cr3); prints("  eip: 0x"); printHexDword(p->ctx.eip); prints("\n");
    prints("eflags: 0x"); printHexDword(p->ctx.eflags); prints("\n");
    prints("es: 0x"); printHexWord(p->ctx.es); prints("  cs: 0x"); printHexWord(p->ctx.cs); prints("\n");
    prints("ss: 0x"); printHexWord(p->ctx.ss); prints("  ds: 0x"); printHexWord(p->ctx.ds); prints("\n");
    prints("fs: 0x"); printHexWord(p->ctx.fs); prints("  gs: 0x"); printHexWord(p->ctx.gs); prints("\n");

    //Get the error code
    prints("Error code: 0x"); printHexDword(p->ctx.err); prints("\n");

    //Get the command byte that the processor failed on:
    prints("Current instructions (0x"); printHexDword((unsigned int)insPtr); prints("): 0x"); printHexByte(insPtr[0]);
    prints(", 0x"); printHexByte(insPtr[1]);
    prints(", 0x"); printHexByte(insPtr[2]);
    prints(", 0x"); printHexByte(insPtr[3]);
    prints(", 0x"); printHexByte(insPtr[4]);
    prints("\n");
}


//This needs to be moved to its own module eventually
void V86Entry(void) {

    unsigned short seg, off;
    unsigned short* stack = (unsigned short*)((unsigned int)0 + (p->ctx.ss << 4) + (p->ctx.esp & 0xFFFF));
    unsigned int* stack32 = (unsigned int*)stack;
    char op32 = 0;

    while(1) {

        switch(insPtr[0]) {

            //INT X
            case 0xCD:
                //If it was a service call, forward it to the syscall handler
                if(insPtr[1] == 0xFF) {

    	    	    prints("Syscall triggered\n");
                    syscall_number = p->ctx.eax & 0xFFFF;
                    syscall_param1 = p->ctx.ebx & 0xFFFF;
                    syscall_param2 = p->ctx.ecx & 0xFFFF;
                    syscall_exec();
                } else {

            		stack -= 3;
            		p->ctx.esp = ((p->ctx.esp & 0xFFFF) - 6) & 0xFFFF;
            		stack[0] = (unsigned short)(p->ctx.eip + 2);
            		stack[1] = p->ctx.cs;
            		stack[2] = (unsigned short)p->ctx.eflags;
            		prints("Stack:\n");
        	        prints("0: 0x"); printHexWord(stack[0]); prints("\n");
        	        prints("1: 0x"); printHexWord(stack[1]); prints("\n");
        	        prints("2: 0x"); printHexWord(stack[2]); prints("\n");

            		if(p->ctx.vif)
            		    stack[2] |= 0x20;
            		else
            		    stack[2] &= 0xFFDF;

                    intVect = 0x00000000 + (insPtr[1] & 0xFF);
                    intVect *= 4;
                    off = ((unsigned short*)intVect)[0];
                    seg = ((unsigned short*)intVect)[1];
                    prints("V86 Interrupt #"); printHexByte(insPtr[1]);
                    prints(" -> "); printHexWord(seg);
                    prints(":"); printHexWord(off);
                    prints("\n");
                    kernelDebug();
    		        //scans(5, fake);
                    p->ctx.cs = seg;
                    p->ctx.eip = (((unsigned int)off) & 0xFFFF);
                }
                return;
                break;

            //IRET
            case 0xCF:
                prints("Return from previous interrupt\n");
        	    prints("Stack:\n");
        	    prints("0: 0x"); printHexWord(stack[0]); prints("\n");
        	    prints("1: 0x"); printHexWord(stack[1]); prints("\n");
        	    prints("2: 0x"); printHexWord(stack[2]); prints("\n");
        	    kernelDebug();
        	    //scans(5, fake);
                p->ctx.eip = stack[0];
        	    p->ctx.cs = stack[1];
        	    p->ctx.eflags = stack[2] | 0x20020;
        	    p->ctx.vif = (stack[2] & 0x20) != 0;
        	    p->ctx.esp = ((p->ctx.esp & 0xFFFF) + 6) & 0xFFFF;
                op32 = 0;
                return;
                break;

            //O32
            case 0x66:
                prints("o32 ");
                (char*)(((((unsigned int)p->ctx.cs)&0xFFFF) << 4) + (((unsigned int)(++p->ctx.eip) &0xFFFF)));
                op32 = 1;
                break;

            //A32
            case 0x67:
                prints("a32 ");
                (char*)(((((unsigned int)p->ctx.cs)&0xFFFF) << 4) + (((unsigned int)(++p->ctx.eip) &0xFFFF)));
                op32 = 1;
                break;

    	    //PUSHF
            case 0x9C:
    	        prints("Flags pushed\n");

                if(op32) {
                    p->ctx.esp = ((p->ctx.esp & 0xFFFF) - 4) & 0xFFFF;
                    stack32--;
    		        stack32[0] = (unsigned short)p->ctx.eflags & 0xDFF;

    		        if(p->ctx.vif)
    		            stack32[0] |= 0x20;
    		        else
    		            stack32[0] &= 0xFFFFFFDF;
                    op32 = 0;
                } else {
                    p->ctx.esp = ((p->ctx.esp & 0xFFFF) - 2) & 0xFFFF;
    		        stack--;
    		        stack[0] = (unsigned short)p->ctx.eflags;

    		        if(p->ctx.vif)
    		            stack[0] |= 0x20;
    		        else
    		            stack[0] &= 0xFFDF;
                }

                p->ctx.eip++;
                return;
                break;

        	//POPF
        	case 0x9D:
        	    prints("Flags popped\n");

                if(op32) {
                    p->ctx.eflags = 0x20020 | (stack32[0] & 0xDFF);
        		    p->ctx.vif = (stack32[0] & 0x20) != 0;
        			p->ctx.esp = ((p->ctx.esp & 0xFFFF) + 4) & 0xFFFF;
                    op32 = 0;
                } else {
        		    p->ctx.eflags = 0x20020 | stack[0];
        		    p->ctx.vif = (stack[0] & 0x20) != 0;
        			p->ctx.esp = ((p->ctx.esp & 0xFFFF) + 2) & 0xFFFF;
                }

        	    p->ctx.eip++;
        	    return;
        	    break;

        	//OUT DX AL
        	case 0xEE:
        	    prints("Out\n");
        	    outb((unsigned short)p->ctx.edx, (unsigned char)p->ctx.eax);
        	    p->ctx.eip++;
        	    return;
        	    break;

        	//IN AL DX
        	case 0xEC:
        	    prints("In\n");
        	    p->ctx.eax &= 0xFFFFFF00;
        	    p->ctx.eax |= ((unsigned int)0 + (inb((unsigned short)p->ctx.edx) & 0xFF));
        	    p->ctx.eip++;
        	    return;
        	    break;

        	//OUT DX AX
        	case 0xEF:
        	    prints("OutW\n");
                if(op32) {
                    outd((unsigned short)p->ctx.edx, p->ctx.eax);
                } else {
        	        outw((unsigned short)p->ctx.edx, (unsigned short)p->ctx.eax);
        	    }
                p->ctx.eip++;
        	    return;
        	    break;

            //IN AX DX
        	case 0xED:
        	    prints("In\n");

                if(op32) {
        			p->ctx.eax = ind(p->ctx.edx);
        		} else {
        			p->ctx.eax &= 0xFFFF0000;
        			p->ctx.eax |= ((unsigned int)0 + (inw((unsigned short)p->ctx.edx) & 0xFFFF));
        		}
        	    p->ctx.eip++;
        	    return;
        	    break;

            //INT 3 (debug) or anything else
            case 0xCC:
                prints("V86 Debug Interrupt\n");
                kernelDebug();
                //scans(5, fake);
                p->ctx.eip++;
                return;
                break;

    		//CLI
    		case 0xfa:
                prints("cli\n");
                p->ctx.vif = 0;
                p->ctx.eip++;
                return;
                break;

    		//STI
            case 0xfb:
                prints("sti\n");
                p->ctx.vif = 1;
                p->ctx.eip++;
                return;
                break;

            default:
                //prints("Unhandled opcode 0x"); printHexByte(insPtr[0]); prints("\n");
                //while(1);
                break;
        }
    }
}


void kernelEntry(void) {

    //Backup the running context
    p->ctx.esp = old_esp;
    p->ctx.cr3 = old_cr3;
    p->ctx.eip = old_eip;
    p->ctx.eflags = old_eflags;
    p->ctx.eax = old_eax;
    p->ctx.ecx = old_ecx;
    p->ctx.edx = old_edx;
    p->ctx.ebx = old_ebx;
    p->ctx.ebp = old_ebp;
    p->ctx.esi = old_esi;
    p->ctx.edi = old_edi;
    p->ctx.es = old_es;
    p->ctx.cs = old_cs;
    p->ctx.ss = old_ss;
    p->ctx.ds = old_ds;
    p->ctx.fs = old_fs;
    p->ctx.gs = old_gs;
    p->ctx.err = old_err;

    switch(except_num) {

        case EX_GPF:
        //Switch to the V86 monitor if the thread was a V86 thread
            if(old_eflags & 0x20000) {

                insPtr = (char*)(((((unsigned int)p->ctx.cs)&0xFFFF) << 4) + (((unsigned int)p->ctx.eip) &0xFFFF));
                V86Entry();
            } else {

                //Otherwise, for now we just dump the system state and move on
                insPtr = (char*)p->ctx.eip;
                prints("(Non-V86)\n");
                kernelDebug();
                scans(5, fake);
            }
            break;

        case EX_SYSCALL:
            syscall_number = p->ctx.eax;
            syscall_param1 = p->ctx.ebx;
            syscall_param2 = p->ctx.ecx;
            syscall_exec();
            break;

        default:
            prints("Interrupt #0x"); printHexByte(except_num); prints(" triggered\n");
            while(1);
            break; 
    }

    returnToProcess(p);
}


void terminateProcess(proc* p) {

    //This is just some bullshit 'go back to console' placeholder
    sys_console();
}


void clearContext(context* ctx) {

    int i;
    unsigned char* buf = (unsigned char*)ctx;

    for(i = 0; i < sizeof(context); i++)
        buf[i] = 0;
}


process* newProcess() {
    
    process* p;
    int i;
        
    //fast-forward to the next free slot
    for(i = 0; procTable[i].id && (i < 256); i++);
    
    if(i == 256)
        return (process*)0x0;
        
    p = &(procTable[i]);
    p->id = nextProc++;
    p->root_page = (pageRange*)0x0;    
    return p;
}


process* newUserProc() {

    process* newP;
    
    newP = newProcess();
    
    if(!newP)
        return newP;
    
    newP->base = 0xB00000;
    newP->size = 0x0;
    
    clearContext(&(newP->ctx));
    newP->ctx.esp = 0xB00FFF;
    newP->ctx.ss = 0x23;
    newP->ctx.ds = 0x23;
    newP->ctx.cs = 0x1B;
    newP->ctx.eip = 0xB01000;

    //Interrupts enabled by default
    newP->ctx.vif = 1;
    newP->ctx.eflags = 0x20;
    return newP;
}

/*
process* newV86Proc() {

    clearContext(&V86Context);
    V86Context.type = PT_V86;

    //Set stack to 0x91000, aka 9000:1000
    V86Context.esp = 0x1000;
    V86Context.ss = 0x9000;
    V86Context.ds = 0x9000;

    //Set the flags to V86 mode enable
    V86Context.eflags = 0x20000;

    //Interrupts enabled by default
    V86Context.vif = 1;
    V86Context.eflags |= 0x20;
    return &V86Context;
}
*/

void setProcEntry(process* p, void* entryPoint) {

    if(p->ctx.type == PT_V86) {

	//Convert entry address to seg:offset
        p->ctx.eip = (unsigned int)entryPoint & 0xFFFF;
        p->ctx.cs = ((unsigned int)entryPoint - p->ctx.eip) >> 4;
    } else {
        p->ctx.eip = (unsigned int)entryPoint;
    }
}


void startProc(process* proc) {

    //Turn off the page mapping of the last process
    if(p) disable_page_range(p->root_page);

    //Enter the new context, assuming the standard
    //user process base address of 0xB00000
    p = proc;
    prints("Applying process paging:\n");
    apply_page_range(p->base, p->root_page);
    returnToProcess(p);
    return;
}


//Kernel id = 1
//id == 0 means no process in this slot
void startProcessManagement() {

    int i;
    p = (process*)0;    
        
    //Clear the contents of the process table
    for(i = 0; i < 256; i++) 
        procTable[i].id = 0;
    
    nextProc = 1;
}


//Append a new page to the end of the process's allocated virtual space
int request_new_page(process* proc) {

    int newSize;
    
    if(!(newSize = append_page(proc->root_page)))
        return proc->size = newSize;
    else
        return newSize;
}


process* exec_process(unsigned char* path) {

    FILE exeFile;
    process* proc;
    char* usrBase = (char*)0xB01000;
    int tmpVal, i;
    int pageCount;
    
    if(!(proc = newUserProc()))
        return proc;
    
    file_open(path, &exeFile);
    
    if(!exeFile.id) {
    
        prints("Could not open file ");
        prints(path);
        prints("\n");
        //fclose(&exeFile);
        //freeProcess(proc);
        return;
    }

    for(i = 0; file_readb(&exeFile) != EOF; i++);
    
    //Add an extra 4k to account for the stack space
    pageCount = (i + 0x1000) / 0x1000;
    
    if(i % 0x1000)
        pageCount++;
        
    proc->root_page = new_page_tree(pageCount);
    
    if(!(proc->root_page)) {
    
        //fclose(&exeFile);
        //freeProcess(proc);
    }
    
    //Finish the definition of the root malloc block
    proc->size = pageCount << 12;
    
    //Activate the new pages so we're writing to the
    //correct locations on physical ram
    apply_page_range(proc->base, proc->root_page);
    
    i = 0;
    while((tmpVal = file_readb(&exeFile)) != EOF)
        usrBase[i++] = (char)tmpVal;

    prints("Launching usermode process\n");
    startProc(proc);
}
