#include "../core/kernel.h"
#include "process.h"
#include "../ascii_io/ascii_o.h"
#include "../ascii_io/ascii_i.h"
#include "../core/syscall.h"
#include "../core/util.h"
#include "../core/expt.h"
#include "../fs/fs.h"
#include "../core/global.h"
#include "../timer/timer.h"
#include "../memory/paging.h"
#include "message.h"


unsigned char fake[6];
unsigned char* insPtr;
context V86Context, usrContext;
int intVect = 0;
int nextProc = 0;
unsigned char procPtr = 0;
unsigned int t_count = 0;

//We'll ACTUALLY use this in the future
process* p = (process*)0;

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


void returnToProcess(process* proc) {

    process* oldP = p;
        
    //needs_swap = 1;
    if(needs_swap) {
                   
        for(++procPtr; (!procTable[procPtr].id); procPtr++);

        proc = &procTable[procPtr];
        needs_swap = 0;
    }
    
    p = proc;
    
    //Turn off the page mapping of the last process
    if(oldP)
        if(oldP->root_page)
            disable_page_range(oldP->base, oldP->root_page);   
    
    DEBUG("Entering process #"); DEBUG_HD(p->id); DEBUG("\n");
    DEBUG("Applying process paging:\n");
    
    if(p->root_page) 
        apply_page_range(p->base, p->root_page, p->flags & PF_SUPER);
    
    //If the process is in debug mode, set the debug flag
    if(p->flags & PF_DEBUG)
        p->ctx.eflags |= 0x100;
    
    prc_is_super = p->flags & PF_SUPER ? 1 : 0;
        
    
    //Restore the running context
    old_esp = p->ctx.esp;
    old_cr3 = p->ctx.cr3;
    old_eip = p->ctx.eip;
    old_eflags = p->ctx.eflags;
    old_eax = p->ctx.eax;
    old_ecx = p->ctx.ecx;
    old_edx = p->ctx.edx;
    old_ebx = p->ctx.ebx;
    old_ebp = p->ctx.ebp;
    old_esi = p->ctx.esi;
    old_edi = p->ctx.edi;
    old_es = p->ctx.es;
    old_cs = p->ctx.cs;
    old_ss = p->ctx.ss;
    old_ds = p->ctx.ds;
    old_fs = p->ctx.fs;
    old_gs = p->ctx.gs;
    old_err = p->ctx.err;
                
    returnToProc();
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

                    syscall_number = p->ctx.eax & 0xFFFF;
                    syscall_param1 = p->ctx.ebx & 0xFFFF;
                    syscall_param2 = p->ctx.ecx & 0xFFFF;
                    syscall_exec();
                    p->ctx.eip += 2;
                } else {

            		stack -= 3;
            		p->ctx.esp = ((p->ctx.esp & 0xFFFF) - 6) & 0xFFFF;
            		stack[0] = (unsigned short)(p->ctx.eip + 2);
            		stack[1] = p->ctx.cs;
            		stack[2] = (unsigned short)p->ctx.eflags;
            		//prints("Stack:\n");
        	        //prints("0: 0x"); printHexWord(stack[0]); prints("\n");
        	        //prints("1: 0x"); printHexWord(stack[1]); prints("\n");
        	        //prints("2: 0x"); printHexWord(stack[2]); prints("\n");

            		if(p->ctx.vif)
            		    stack[2] |= 0x20;
            		else
            		    stack[2] &= 0xFFDF;

                    intVect = 0x00000000 + (insPtr[1] & 0xFF);
                    intVect *= 4;
                    off = ((unsigned short*)intVect)[0];
                    seg = ((unsigned short*)intVect)[1];
                    prints("(V86 Interrupt #"); printHexByte(insPtr[1]);
                    prints(" -> "); printHexWord(seg);
                    prints(":"); printHexWord(off);
                    prints(")");
                    //kernelDebug();
    		        //scans(5, fake);
                    p->ctx.cs = seg;
                    p->ctx.eip = (((unsigned int)off) & 0xFFFF);
                }
                return;
                break;

            //IRET
            case 0xCF:
                prints("Return from previous interrupt\n");
        	    //prints("Stack:\n");
        	    //prints("0: 0x"); printHexWord(stack[0]); prints("\n");
        	    //prints("1: 0x"); printHexWord(stack[1]); prints("\n");
        	    //prints("2: 0x"); printHexWord(stack[2]); prints("\n");
        	    //kernelDebug();
        	    //scans(5, fake);
                p->ctx.eip = stack[0];
        	    p->ctx.cs = stack[1];
        	    p->ctx.eflags = stack[2] | 0x20020;
        	    p->ctx.vif = (stack[2] & 0x20) != 0;
        	    p->ctx.esp = ((p->ctx.esp & 0xFFFF) + 6) & 0xFFFF;
                return;
                break;

            //O32
            case 0x66:
                prints("(o32)");
                insPtr = (char*)(((((unsigned int)p->ctx.cs)&0xFFFF) << 4) + (((unsigned int)(++p->ctx.eip) &0xFFFF)));
                op32 = 1;
                break;

            //A32
            case 0x67:
                prints("(a32)");
                insPtr = (char*)(((((unsigned int)p->ctx.cs)&0xFFFF) << 4) + (((unsigned int)(++p->ctx.eip) &0xFFFF)));
                op32 = 1;
                break;

    	    //PUSHF
            case 0x9C:
    	        prints("(F Pu)");

                if(op32) {
                    p->ctx.esp = ((p->ctx.esp & 0xFFFF) - 4) & 0xFFFF;
                    stack32--;
    		        stack32[0] = (unsigned short)p->ctx.eflags & 0xDFF;

    		        if(p->ctx.vif)
    		            stack32[0] |= 0x20;
    		        else
    		            stack32[0] &= 0xFFFFFFDF;
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
        	    prints("(F Po)");

                if(op32) {
                    p->ctx.eflags = 0x20020 | (stack32[0] & 0xDFF);
        		    p->ctx.vif = (stack32[0] & 0x20) != 0;
        			p->ctx.esp = ((p->ctx.esp & 0xFFFF) + 4) & 0xFFFF;
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
        	    prints("(Out)");
        	    outb((unsigned short)p->ctx.edx, (unsigned char)p->ctx.eax);
        	    p->ctx.eip++;
        	    return;
        	    break;

        	//IN AL DX
        	case 0xEC:
        	    prints("(In)");
        	    p->ctx.eax &= 0xFFFFFF00;
        	    p->ctx.eax |= ((unsigned int)0 + (inb((unsigned short)p->ctx.edx) & 0xFF));
        	    p->ctx.eip++;
        	    return;
        	    break;

        	//OUT DX AX
        	case 0xEF:
        	    prints("(OutW)");
                //if(op32) {
                //    outd((unsigned short)p->ctx.edx, p->ctx.eax);
                //} else {
        	        outw((unsigned short)p->ctx.edx, (unsigned short)p->ctx.eax);
        	    //}
                p->ctx.eip++;
        	    return;
        	    break;

            //IN AX DX
        	case 0xED:
        	    prints("(InW)");

                //if(op32) {
        		//	p->ctx.eax = ind(p->ctx.edx);
        		//} else {
        			p->ctx.eax &= 0xFFFF0000;
        			p->ctx.eax |= ((unsigned int)0 + (inw((unsigned short)(p->ctx.edx&0xFFFF)) & 0xFFFF));
        		//}
        	    p->ctx.eip++;
        	    return;
        	    break;

            //INT 3 (debug) or anything else
            case 0xCC:
                prints("(V86 Debug Interrupt)");
                kernelDebug();
                scans(5, fake);
                p->ctx.eip++;
                return;
                break;

    		//CLI
    		case 0xfa:
                prints("(cli)");
                p->ctx.vif = 0;
                p->ctx.eip++;
                return;
                break;

    		//STI
            case 0xfb:
                prints("(sti)");
                p->ctx.vif = 1;
                p->ctx.eip++;
                return;
                break;

            default:
                prints("(!!0x"); printHexByte(insPtr[0]); prints("!!)");
                while(1);
                break;
        }
    }
}


void kernelEntry(void) {

    unsigned int kflags;
    unsigned short* stack;
    
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
                
    if(p->flags & PF_V86)
        insPtr = (char*)(((((unsigned int)p->ctx.cs)&0xFFFF) << 4) + (((unsigned int)p->ctx.eip) &0xFFFF));
    else
        insPtr = (char*)p->ctx.eip;            
                
    switch(except_num) {
    
        case EX_GPF:
        //Switch to the V86 monitor if the thread was a V86 thread
            if(p->flags & PF_V86) {

                V86Entry();
            } else {

                //Otherwise, for now we just dump the system state and move on
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
        
        case FORCE_ENTER:
            //We don't want to do anything here, this is just
            //so that we get an entry into and an exit from the kernel
            break;
        
        case EX_DEBUGCALL:
            stack = (unsigned short*)((unsigned int)0 + (p->ctx.ss << 4) + (p->ctx.esp & 0xFFFF));
            prints("Process #"); printHexDword(p->id); prints(" single-step\n");
            prints("Stack:\n");
        	prints("   0: 0x"); printHexWord(stack[0]); prints("\n");
        	prints("   1: 0x"); printHexWord(stack[1]); prints("\n");
        	prints("   2: 0x"); printHexWord(stack[2]); prints("\n");
            prints("   3: 0x"); printHexWord(stack[3]); prints("\n");
        	prints("   4: 0x"); printHexWord(stack[4]); prints("\n");
        	prints("   5: 0x"); printHexWord(stack[5]); prints("\n");
            kernelDebug();
            scans(5, fake);
            break;
        
        default:
            prints("Interrupt #0x"); printHexByte(except_num); prints(" triggered\n");
            kernelDebug();
            scans(5, fake);
            break; 
    }

    returnToProcess(p);
}


void endProc(process* proc) {

    disable_page_range(0xB00000, proc->root_page);
    del_page_tree(proc->root_page);
    proc->root_page = (pageRange*)0;
    proc->id = 0;    
    
    if(proc == p)
        needs_swap = 1;
    
    returnToProcess(p);
}


void clearContext(context* ctx) {

    int i;
    unsigned char* buf = (unsigned char*)ctx;

    for(i = 0; i < sizeof(context); i++)
        buf[i] = 0;
}


process* newProcess() {
    
    process* proc;
    int i;
        
    //fast-forward to the next free slot
    for(i = 0; procTable[i].id && (i < 256); i++);
    
    if(i == 256)
        return (process*)0x0;
        
    proc = &(procTable[i]);
    procPtr = (unsigned char)i;
    proc->id = nextProc++;
    proc->root_page = (pageRange*)0x0;  
    proc->root_msg = (message*)0x0;
    proc->flags = 0;
    return proc;
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
    newP->ctx.eflags = 0x200;
    return newP;
}


process* newSuperProc() {

    process* newP;
    
    newP = newProcess();
        
    if(!newP)
        return newP;
    
    //Set the superproc bit
    newP->flags |= PF_SUPER;
    
    newP->base = 0xB00000;
    newP->size = 0x0;
    
    clearContext(&(newP->ctx));
    newP->ctx.esp = 0xB00FFF;
    newP->ctx.ss = 0x10;
    newP->ctx.ds = 0x10;
    newP->ctx.cs = 0x8;
    newP->ctx.eip = 0xB01000;

    //Interrupts enabled by default
    newP->ctx.vif = 1;
    newP->ctx.eflags = 0x200;
    return newP;
}


process* newV86Proc() {

    process* newP;
    
    newP = newProcess();
        
    if(!newP)
        return newP;
    
    //Set the v86 mode bit
    newP->flags |= PF_V86;
    
    newP->base = 0xB00000;
    newP->size = 0x0;
    
    clearContext(&(newP->ctx));
    newP->ctx.esp = 0x1000;
    newP->ctx.ss = 0x9000;
    newP->ctx.ds = 0x9000;
    newP->ctx.cs = 0x8000;
    newP->ctx.eip = 0x0;

    //Interrupts enabled by default
    newP->ctx.vif = 1;
    newP->ctx.eflags = 0x20200;
    return newP;
}


void setProcEntry(process* p, void* entryPoint) {

    if(p->ctx.type == PT_V86) {

	//Convert entry address to seg:offset
        p->ctx.eip = (unsigned int)entryPoint & 0xFFFF;
        p->ctx.cs = ((unsigned int)entryPoint - p->ctx.eip) >> 4;
    } else {
        p->ctx.eip = (unsigned int)entryPoint;
    }
}


void enterProc(unsigned int pid) {

    int i;
    process* proc;

    for(i = 0; i < 256 && (procTable[i].id != pid); i++);
    
    if(i == 256)
        return;
    
    proc = &procTable[i];
    
    //Enter the new context
    needs_swap = 0;
    returnToProcess(proc);
}


//Kernel id = 1
//id == 0 means no process in this slot
void startProcessManagement() {

    int i;
    p = (process*)0;    
    needs_swap = 0;
    procTable = (process*)0x2029A0;
        
    //Clear the contents of the process table
    for(i = 0; i < 256; i++) 
        procTable[i].id = 0;
    
    nextProc = 1;
    procPtr = 0;
}


//Append a new page to the end of the process's allocated virtual space
int request_new_page(process* proc) {

    int newSize;
    
    if(!(newSize = append_page(proc->root_page)))
        return proc->size = newSize;
    else
        return newSize;
}


unsigned int exec_v86(unsigned char* path) {

    FILE exeFile, exeFile2;
    process* proc;
    char* usrBase = (char*)0x80000;
    int tmpVal, i;
    int pageCount;
    unsigned char pathBuf[255];
    
    for(i = 0; path[i]; i++)
        pathBuf[i] = path[i];
        
    pathBuf[i] = 0;
       
    if(!(proc = newV86Proc()))
        return 0;
    
    file_open(pathBuf, &exeFile);
    
    if(!exeFile.id) {
    
        prints("Could not open file ");
        prints(path);
        prints("\n");
        //fclose(&exeFile);
        //freeProcess(proc);
        return 0;
    }

    for(i = 0; file_readb(&exeFile) != EOF; i++);
    
    //Finish the definition of the root malloc block
    proc->size = i;
    
    //Because we can't rewind or close and reopen the file yet
    file_open(pathBuf, &exeFile2);
    i = 0;
    while((tmpVal = file_readb(&exeFile2)) != EOF)
        usrBase[i++] = (char)tmpVal;
    
    prints("Launching v86 process\n");    
        
    return proc->id;
}


unsigned int exec_process(unsigned char* path, char isSuper) {

    FILE exeFile, exeFile2;
    process* proc;
    char* usrBase = (char*)0xB01000;
    int tmpVal, i;
    int pageCount;
    unsigned char pathBuf[255];
    
    for(i = 0; path[i]; i++)
        pathBuf[i] = path[i];
        
    pathBuf[i] = 0;
    
    if(isSuper) {
    
        if(!(proc = newSuperProc()))
            return 0;
    } else {
    
        if(!(proc = newUserProc()))
            return 0;
    }
    
    file_open(pathBuf, &exeFile);
    
    if(!exeFile.id) {
    
        prints("Could not open file ");
        prints(path);
        prints("\n");
        //fclose(&exeFile);
        //freeProcess(proc);
        return 0;
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
    apply_page_range(proc->base, proc->root_page, proc->flags & PF_SUPER);
    
    //Because we can't rewind or close and reopen the file yet
    file_open(pathBuf, &exeFile2);
    i = 0;
    while((tmpVal = file_readb(&exeFile2)) != EOF)
        usrBase[i++] = (char)tmpVal;
    
    //Restore the active process's paging
    if(p) apply_page_range(p->base, p->root_page, p->flags & PF_SUPER);
    
    prints("Launching usermode process\n");    
        
    return proc->id;
}
