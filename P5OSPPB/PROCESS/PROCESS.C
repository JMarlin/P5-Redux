#include "../core/kernel.h"
#include "process.h"
#include "../ascii_io/ascii_o.h"
#include "../ascii_io/ascii_i.h"
#include "../core/syscall.h"
#include "../core/util.h"


unsigned char fake[6];
unsigned char* insPtr;
context V86Context, usrContext;
unsigned short V86RetCS;
unsigned int V86RetIP;
int intVect = 0;
int op32 = 0;

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
    unsigned short* stack = (unsigned short*)((unsigned int)0 + (activeContext->ss << 4) + (activeContext->esp & 0xFFFF));
    unsigned int* stack32 = (unsigned int*)stack;

    switch(insPtr[0]) {

        //INT X
        case 0xCD:
            //If it was a service call, forward it to the syscall handler
            if(insPtr[1] == 0xFF) {

	    	    prints("Syscall triggered\n");
                syscall_number = activeContext->eax & 0xFFFF;
                syscall_param1 = activeContext->ebx & 0xFFFF;
                syscall_param2 = activeContext->ecx & 0xFFFF;
                syscall_exec();
            } else {

        		stack -= 3;
        		activeContext->esp = ((activeContext->esp & 0xFFFF) - 6) & 0xFFFF;
        		stack[0] = (unsigned short)(activeContext->eip + 2);
        		stack[1] = activeContext->cs;
        		stack[2] = (unsigned short)activeContext->eflags;
        		prints("Stack:\n");
    	        prints("0: 0x"); printHexWord(stack[0]); prints("\n");
    	        prints("1: 0x"); printHexWord(stack[1]); prints("\n");
    	        prints("2: 0x"); printHexWord(stack[2]); prints("\n");

        		if(activeContext->vif)
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
                activeContext->cs = seg;
                activeContext->eip = (((unsigned int)off) & 0xFFFF);
            }
            op32 = 0;
            returnToProcess(activeContext);
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
            activeContext->eip = stack[0];
    	    activeContext->cs = stack[1];
    	    activeContext->eflags = stack[2] | 0x20020;
    	    activeContext->vif = (stack[2] & 0x20) != 0;
    	    activeContext->esp = ((activeContext->esp & 0xFFFF) + 6) & 0xFFFF;
            op32 = 0;
            returnToProcess(activeContext);
            break;

        //O32
        case 0x66:
            prints("o32 ");
            op32 = 1;
            activeContext->eip++;
            returnToProcess(activeContext);
            break;

        //A32
        case 0x67:
            prints("a32 ");
            activeContext->eip++;
            op32 = 0;
            returnToProcess(activeContext);
            break;

	//PUSHF
        case 0x9C:
	        prints("Flags pushed\n");

            if(op32) {
                activeContext->esp = ((activeContext->esp & 0xFFFF) - 4) & 0xFFFF;
                stack32--;
		        stack32[0] = (unsigned short)activeContext->eflags & 0xDFF;

		        if(activeContext->vif)
		            stack32[0] |= 0x20;
		        else
		            stack32[0] &= 0xFFFFFFDF;
                op32 = 0;
            } else {
                activeContext->esp = ((activeContext->esp & 0xFFFF) - 2) & 0xFFFF;
		        stack--;
		        stack[0] = (unsigned short)activeContext->eflags;

		        if(activeContext->vif)
		            stack[0] |= 0x20;
		        else
		            stack[0] &= 0xFFDF;
            }

            activeContext->eip++;
            returnToProcess(activeContext);
            break;

	//POPF
	case 0x9D:
	    prints("Flags popped\n");

        if(op32) {
            activeContext->eflags = 0x20020 | (stack32[0] & 0xDFF);
		    activeContext->vif = (stack32[0] & 0x20) != 0;
			activeContext->esp = ((activeContext->esp & 0xFFFF) + 4) & 0xFFFF;
            op32 = 0;
        } else {
		    activeContext->eflags = 0x20020 | stack[0];
		    activeContext->vif = (stack[0] & 0x20) != 0;
			activeContext->esp = ((activeContext->esp & 0xFFFF) + 2) & 0xFFFF;
        }

	    activeContext->eip++;
	    returnToProcess(activeContext);
	    break;

	//OUT DX AL
	case 0xEE:
	    prints("Out\n");
	    outb((unsigned short)activeContext->edx, (unsigned char)activeContext->eax);
	    activeContext->eip++;
        op32 = 0;
	    returnToProcess(activeContext);
	    break;

	//IN AL DX
	case 0xEC:
	    prints("In\n");
	    activeContext->eax &= 0xFFFFFF00;
	    activeContext->eax |= ((unsigned int)0 + (inb((unsigned short)activeContext->edx) & 0xFF));
	    activeContext->eip++;
        op32 = 0;
	    returnToProcess(activeContext);
	    break;

	//OUT DX AX
	case 0xEF:
	    prints("OutW\n");
        if(op32) {
            outd((unsigned short)activeContext->edx, activeContext->eax);
        } else {
	        outw((unsigned short)activeContext->edx, (unsigned short)activeContext->eax);
	    }
        activeContext->eip++;
        op32 = 0;
	    returnToProcess(activeContext);
	    break;

    //IN AX DX
	case 0xED:
	    prints("In\n");

        if(op32) {
			activeContext->eax = ind(activeContext->edx);
		} else {
			activeContext->eax &= 0xFFFF0000;
			activeContext->eax |= ((unsigned int)0 + (inw((unsigned short)activeContext->edx) & 0xFFFF));
		}
	    activeContext->eip++;
        op32 = 0;
	    returnToProcess(activeContext);
	    break;

        //INT 3 (debug) or anything else
        case 0xCC:
            prints("V86 Debug Interrupt\n");
            kernelDebug();
            //scans(5, fake);
            activeContext->eip++;
            op32 = 0;
            returnToProcess(activeContext);
            break;

		//CLI
		case 0xfa:
            prints("cli\n");
            activeContext->vif = 0;
            activeContext->eip++;
            op32 = 0;
            returnToProcess(activeContext);
            break;

		//STI
        case 0xfb:
            prints("sti\n");
            activeContext->vif = 1;
            activeContext->eip++;
            op32 = 0;
            returnToProcess(activeContext);
            break;

        default:
            prints("Unhandled opcode 0x"); printHexByte(insPtr[0]); prints("\n");
            while(1);
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
        prints("(Non-V86)\n");
        kernelDebug();
        scans(5, fake);
        returnToProcess(activeContext);
    }
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


context* newUserProc() {

    clearContext(&usrContext);
    usrContext.esp = 0x800FFF;
    usrContext.ss = 0x23;
    usrContext.ds = 0x23;
    usrContext.cs = 0x1B;

    //Interrupts enabled by default
    usrContext.vif = 1;
    usrContext.eflags = 0x20;
    return &usrContext;
}


context* newV86Proc() {

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


void setProcEntry(context* ctx, void* entryPoint) {

    if(ctx->type == PT_V86) {

	//Convert entry address to seg:offset
        ctx->eip = (unsigned int)entryPoint & 0xFFFF;
        ctx->cs = ((unsigned int)entryPoint - ctx->eip) >> 4;
    } else {
        ctx->cs = (unsigned int)entryPoint;
    }
}


void startProc(context* ctx) {

    //Enter the new context
    activeContext = ctx;
    returnToProcess(activeContext);
}
