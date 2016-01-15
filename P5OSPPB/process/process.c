#include "../core/kernel.h"
#include "../core/irq.h"
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
#include "../memory/memory.h"
#include "message.h"

unsigned char fake[6];
unsigned char* insPtr;
context V86Context, usrContext;
int intVect = 0;
int nextProc = 0;
unsigned int swap_count = 0;
unsigned int entry_count = 0;
unsigned char procPtr = 0;
unsigned int t_count = 0;
unsigned char needs_swap = 0;

//extern unsigned char _prc_is_v86;
extern unsigned char _was_spurious;

//We'll ACTUALLY use this in the future
process* p = (process*)0;
process* proc_backup = (process*)0;
unsigned char exeption_backup = 0;

void entry_debug(void) {

    //Kernel debug, should be moved to its own function
    prints("ABOUT TO ENTER PROCESS\n");
    prints("Previous State:\n");
    prints("eax: 0x"); printHexDword(_old_eax); prints("  ebx: 0x"); printHexDword(_old_ebx); prints("\n");
    prints("ecx: 0x"); printHexDword(_old_ecx); prints("  edx: 0x"); printHexDword(_old_edx); prints("\n");
    prints("esp: 0x"); printHexDword(_old_esp); prints("  ebp: 0x"); printHexDword(_old_ebp); prints("\n");
    prints("esi: 0x"); printHexDword(_old_esi); prints("  edi: 0x"); printHexDword(_old_edi); prints("\n");
    prints("cr3: 0x"); printHexDword(_old_cr3); prints("  eip: 0x"); printHexDword(_old_eip); prints("\n");
    prints("eflags: 0x"); printHexDword(_old_eflags); prints("\n");
    prints("es: 0x"); printHexWord(_old_es); prints("  cs: 0x"); printHexWord(_old_cs); prints("\n");
    prints("ss: 0x"); printHexWord(_old_ss); prints("  ds: 0x"); printHexWord(_old_ds); prints("\n");
    prints("fs: 0x"); printHexWord(_old_fs); prints("  gs: 0x"); printHexWord(_old_gs); prints("\n");

    //Get the error code
    prints("Error code: 0x"); printHexDword(_old_err); prints("\n");

    //Get the command byte that the processor failed on:
    prints("Current instructions (0x"); printHexDword((unsigned int)insPtr); prints("): 0x"); printHexByte(insPtr[0]);
    prints(", 0x"); printHexByte(insPtr[1]);
    prints(", 0x"); printHexByte(insPtr[2]);
    prints(", 0x"); printHexByte(insPtr[3]);
    prints(", 0x"); printHexByte(insPtr[4]);
    prints("\n");
    scans(5, fake);
}

void kernelDebugWithProc(process* dbg_proc) {

    unsigned char* current_pc;

    if(dbg_proc->flags & PF_V86)
        current_pc = (char*)(((((unsigned int)dbg_proc->ctx.cs)&0xFFFF) << 4) + (((unsigned int)dbg_proc->ctx.eip) &0xFFFF));
    else
        current_pc = (char*)dbg_proc->ctx.eip;
        
    prints("Previous State:\n");
    prints("eax: 0x"); printHexDword(dbg_proc->ctx.eax); prints("  ebx: 0x"); printHexDword(dbg_proc->ctx.ebx); prints("\n");
    prints("ecx: 0x"); printHexDword(dbg_proc->ctx.ecx); prints("  edx: 0x"); printHexDword(dbg_proc->ctx.edx); prints("\n");
    prints("esp: 0x"); printHexDword(dbg_proc->ctx.esp); prints("  ebp: 0x"); printHexDword(dbg_proc->ctx.ebp); prints("\n");
    prints("esi: 0x"); printHexDword(dbg_proc->ctx.esi); prints("  edi: 0x"); printHexDword(dbg_proc->ctx.edi); prints("\n");
    prints("cr3: 0x"); printHexDword(dbg_proc->ctx.cr3); prints("  eip: 0x"); printHexDword(dbg_proc->ctx.eip); prints("\n");
    prints("eflags: 0x"); printHexDword(dbg_proc->ctx.eflags); prints("\n");
    prints("es: 0x"); printHexWord(dbg_proc->ctx.es); prints("  cs: 0x"); printHexWord(dbg_proc->ctx.cs); prints("\n");
    prints("ss: 0x"); printHexWord(dbg_proc->ctx.ss); prints("  ds: 0x"); printHexWord(dbg_proc->ctx.ds); prints("\n");
    prints("fs: 0x"); printHexWord(dbg_proc->ctx.fs); prints("  gs: 0x"); printHexWord(dbg_proc->ctx.gs); prints("\n");

    //Get the error code
    prints("Error code: 0x"); printHexDword(dbg_proc->ctx.err); prints("\n");

    //Get the command byte that the processor failed on:
    prints("Current instructions (0x"); printHexDword((unsigned int)current_pc);
    prints("): 0x"); printHexByte(current_pc[0]);
    prints(", 0x"); printHexByte(current_pc[1]);
    prints(", 0x"); printHexByte(current_pc[2]);
    prints(", 0x"); printHexByte(current_pc[3]);
    prints(", 0x"); printHexByte(current_pc[4]);
    prints("\n");
}

void kernelDebug() {

    kernelDebugWithProc(p);
}

void analyzeProcUsage(process* proc) {

    int i;

    proc->called_count++;
    entry_count++;

    if(swap_count == 2000) {

        swap_count = 0;
        entry_count /= 100;

        for(i = 0; i < 255; i++) {

            if(!procTable[i].id)
                continue;

            procTable[i].cpu_pct = procTable[i].called_count / entry_count;
            procTable[i].called_count = 0;
        }

        entry_count = 0;
    }
}

void returnToProcess(process* proc) {

    message temp_message;
    process* oldP = p;

    if(needs_swap) //EG: The timer fired
        swap_count++;

    //needs_swap means that the timer is up
    //PF_WAITMSG means that the current process has been put to sleep
    if(needs_swap || (proc->flags & PF_WAITMSG)) {

        //Find the next availible process, round-robin
        //This ignores uninitialized proc entries and sleeping procs
        for(++procPtr; (!procTable[procPtr].id) || (procTable[procPtr].flags & PF_WAITMSG); procPtr++);

        proc = &procTable[procPtr];
        needs_swap = 0;
    }

    p = proc;

    //Send the pending message if the proc was just woken up
    if(p->flags & PF_WOKENMSG) {

        getMessage(p, &temp_message, p->wait_pid, p->wait_cmd);
        p->ctx.eax = 1; //a 1 in eax is 'message found'
        p->ctx.ebx = temp_message.source;
        p->ctx.ecx = temp_message.command;
        p->ctx.edx = temp_message.payload;

        //Clear the flag
        p->flags &= ~((unsigned int)PF_WOKENMSG);
    }

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

    //If the process is a virtual 8086 process, set its IOPL to 3
    //so it can do port IO without having to fuck with the monitor
    //if(p->flags & PF_V86)
    //    p->ctx.eflags |= 0x3000;

    _prc_is_super = (p->flags & PF_SUPER) ? 1 : 0;
    //_prc_is_v86 = (p->flags & PF_V86) ? 1 : 0;

    //Restore the running context
    _old_esp = p->ctx.esp;
    _old_cr3 = p->ctx.cr3;
    _old_eip = p->ctx.eip;
    _old_eflags = p->ctx.eflags;
    _old_eax = p->ctx.eax;
    _old_ecx = p->ctx.ecx;
    _old_edx = p->ctx.edx;
    _old_ebx = p->ctx.ebx;
    _old_ebp = p->ctx.ebp;
    _old_esi = p->ctx.esi;
    _old_edi = p->ctx.edi;
    _old_es = p->ctx.es;
    _old_cs = p->ctx.cs;
    _old_ss = p->ctx.ss;
    _old_ds = p->ctx.ds;
    _old_fs = p->ctx.fs;
    _old_gs = p->ctx.gs;
    _old_err = p->ctx.err;

    //Do process accounting
    analyzeProcUsage(p);

    __asm__ volatile ("jmp _returnToProc\n");
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

                    KPRINTS("\n[v86 syscall @ ");
                    KPRINTHEXWORD(p->ctx.cs);
                    KPCHAR(':');
                    KPRINTHEXWORD((unsigned short)(p->ctx.eip & 0xFFFF));
                    KPCHAR(']');
                    _syscall_number = p->ctx.eax & 0xFFFF;
                    _syscall_param1 = p->ctx.ebx & 0xFFFF;
                    _syscall_param2 = p->ctx.ecx & 0xFFFF;
                    syscall_exec();
                    p->ctx.eip += 2;
                } else {

                    KPRINTS("\n[v86 INT ");
                    KPRINTHEXBYTE(insPtr[1]);
                    KPRINTS(" @ ");
                    KPRINTHEXWORD(p->ctx.cs);
                    KPCHAR(':');
                    KPRINTHEXWORD((unsigned short)(p->ctx.eip & 0xFFFF));
                    KPCHAR(']');
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
                    p->ctx.cs = seg;
                    p->ctx.eip = (((unsigned int)off) & 0xFFFF);
                }
                return;
                break;

            //IRET
            case 0xCF:
                KPRINTS("\n[v86 IRET @ ");
                KPRINTHEXWORD(p->ctx.cs);
                KPCHAR(':');
                KPRINTHEXWORD((unsigned short)(p->ctx.eip & 0xFFFF));
                KPCHAR(']');
                p->ctx.eip = stack[0];
        	    p->ctx.cs = stack[1];
        	    p->ctx.eflags = stack[2] | 0x20020;
        	    p->ctx.vif = (stack[2] & 0x20) != 0;
        	    p->ctx.esp = ((p->ctx.esp & 0xFFFF) + 6) & 0xFFFF;
                return;
                break;

            //O32
            case 0x66:
                KPRINTS("\n[v86 O32 @ ");
                KPRINTHEXWORD(p->ctx.cs);
                KPCHAR(':');
                KPRINTHEXWORD((unsigned short)(p->ctx.eip & 0xFFFF));
                KPCHAR(']');
                insPtr = (char*)(((((unsigned int)p->ctx.cs)&0xFFFF) << 4) + (((unsigned int)(++p->ctx.eip) &0xFFFF)));
                op32 = 1;
                break;

            //A32
            case 0x67:
                KPRINTS("\n[v86 A32 @ ");
                KPRINTHEXWORD(p->ctx.cs);
                KPCHAR(':');
                KPRINTHEXWORD((unsigned short)(p->ctx.eip & 0xFFFF));
                KPCHAR(']');
                insPtr = (char*)(((((unsigned int)p->ctx.cs)&0xFFFF) << 4) + (((unsigned int)(++p->ctx.eip) &0xFFFF)));
                op32 = 1;
                break;

    	    //PUSHF
            case 0x9C:
    	        KPRINTS("\n[v86 PUSHF @ ");
                KPRINTHEXWORD(p->ctx.cs);
                KPCHAR(':');
                KPRINTHEXWORD((unsigned short)(p->ctx.eip & 0xFFFF));
                KPCHAR(']');

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
        	    KPRINTS("\n[v86 POPF @ ");
                KPRINTHEXWORD(p->ctx.cs);
                KPCHAR(':');
                KPRINTHEXWORD((unsigned short)(p->ctx.eip & 0xFFFF));
                KPCHAR(']');

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

            //INB immediate address
            case 0xE4:
                KPRINTS("\n[v86 INW Imm @ ");
                KPRINTHEXWORD(p->ctx.cs);
                KPCHAR(':');
                KPRINTHEXWORD((unsigned short)(p->ctx.eip & 0xFFFF));
                KPCHAR(']');
                if(op32) {
                    p->ctx.eax = ind((unsigned short)(insPtr[1]));
                } else {
                    p->ctx.eax &= 0xFFFFFF00;
                    p->ctx.eax |= ((unsigned int)0 + (inb((unsigned short)(insPtr[1])) & 0xFF));
                    p->ctx.eip += 2;
                }
                return;
                break;

            //INW/IND immediate address
            case 0xE5:
                KPRINTS("\n[v86 INW Imm @ ");
                KPRINTHEXWORD(p->ctx.cs);
                KPCHAR(':');
                KPRINTHEXWORD((unsigned short)(p->ctx.eip & 0xFFFF));
                KPCHAR(']');
                if(op32) {
                    p->ctx.eax = ind((unsigned short)(insPtr[1]));
                } else {
                    p->ctx.eax &= 0xFFFF0000;
                    p->ctx.eax |= ((unsigned int)0 + (inw((unsigned short)(insPtr[1])) & 0xFFFF));
                }
                p->ctx.eip += 2;
                return;
                break;

            //OUTB immediate address
            case 0xE6:
                KPRINTS("\n[v86 OUTB Imm @ ");
                KPRINTHEXWORD(p->ctx.cs);
                KPCHAR(':');
                KPRINTHEXWORD((unsigned short)(p->ctx.eip & 0xFFFF));
                KPCHAR(']');
                
                if(op32) {
                    outd((unsigned short)(insPtr[1]), p->ctx.eax);
                } else {
                    outb((unsigned short)(insPtr[1]), (unsigned char)p->ctx.eax);
                }
                p->ctx.eip += 2;
                return;
                break;

            //OUTW/OUTD immediate address
            case 0xE7:
                KPRINTS("\n[v86 OUTW Imm @ ");
                KPRINTHEXWORD(p->ctx.cs);
                KPCHAR(':');
                KPRINTHEXWORD((unsigned short)(p->ctx.eip & 0xFFFF));
                KPCHAR(']');
                if(op32) {
                    outd((unsigned short)(insPtr[1]), p->ctx.eax);
                } else {
                    outw((unsigned short)(insPtr[1]), (unsigned short)p->ctx.eax);
                }
                p->ctx.eip += 2;
                return;
                break;

        	//OUT DX AL
        	case 0xEE:
        	    KPRINTS("\n[v86 OUTB @ ");
                KPRINTHEXWORD(p->ctx.cs);
                KPCHAR(':');
                KPRINTHEXWORD((unsigned short)(p->ctx.eip & 0xFFFF));
                KPCHAR(']');
                
                if(op32) {
                    outd((unsigned short)p->ctx.edx, p->ctx.eax);
                } else {
        	       outb((unsigned short)p->ctx.edx, (unsigned char)p->ctx.eax);
                }
        	    p->ctx.eip++;
        	    return;
        	    break;

        	//IN AL DX
        	case 0xEC:
        	    KPRINTS("\n[v86 INB @ ");
                KPRINTHEXWORD(p->ctx.cs);
                KPCHAR(':');
                KPRINTHEXWORD((unsigned short)(p->ctx.eip & 0xFFFF));
                KPCHAR(']');
                
                if(op32) {
        			p->ctx.eax = ind((unsigned short)p->ctx.edx);
        		} else {
            		p->ctx.eax &= 0xFFFFFF00;
            	    p->ctx.eax |= ((unsigned int)0 + (inb((unsigned short)p->ctx.edx) & 0xFF));
                }
        	    p->ctx.eip++;
        	    return;
        	    break;

        	//OUT DX AX
        	case 0xEF:
        	    KPRINTS("\n[v86 OUTW @ ");
                KPRINTHEXWORD(p->ctx.cs);
                KPCHAR(':');
                KPRINTHEXWORD((unsigned short)(p->ctx.eip & 0xFFFF));
                KPCHAR(']');
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
        	    KPRINTS("\n[v86 INW @ ");
                KPRINTHEXWORD(p->ctx.cs);
                KPCHAR(':');
                KPRINTHEXWORD((unsigned short)(p->ctx.eip & 0xFFFF));
                KPCHAR(']');

                if(op32) {
        			p->ctx.eax = ind((unsigned short)p->ctx.edx);
        		} else {
        			p->ctx.eax &= 0xFFFF0000;
        			p->ctx.eax |= ((unsigned int)0 + (inw((unsigned short)(p->ctx.edx&0xFFFF)) & 0xFFFF));
        		}
        	    p->ctx.eip++;
        	    return;
        	    break;

            //INT 3 (debug) or anything else
            case 0xCC:
                KPRINTS("\n[v86 DEBUG @ ");
                KPRINTHEXWORD(p->ctx.cs);
                KPCHAR(':');
                KPRINTHEXWORD((unsigned short)(p->ctx.eip & 0xFFFF));
                KPCHAR(']');
                kernelDebug();
                scans(5, fake);
                p->ctx.eip++;
                return;
                break;

    		//CLI
    		case 0xfa:
                KPRINTS("\n[v86 CLI @ ");
                KPRINTHEXWORD(p->ctx.cs);
                KPCHAR(':');
                KPRINTHEXWORD((unsigned short)(p->ctx.eip & 0xFFFF));
                KPCHAR(']');
                p->ctx.vif = 0;
                p->ctx.eip++;
                return;
                break;

    		//STI
            case 0xfb:
                KPRINTS("\n[v86 STI @ ");
                KPRINTHEXWORD(p->ctx.cs);
                KPCHAR(':');
                KPRINTHEXWORD((unsigned short)(p->ctx.eip & 0xFFFF));
                KPCHAR(']');
                p->ctx.vif = 1;
                p->ctx.eip++;
                return;
                break;

            default:
                //for now, assume that we didn't have an
                //execption but rather just a hardware interrupt or something
                KPRINTS("\n[v86 illegal op ");
                KPRINTHEXBYTE(insPtr[0]);
                KPRINTS(" @ ");
                KPRINTHEXWORD(p->ctx.cs);
                KPCHAR(':');
                KPRINTHEXWORD((unsigned short)(p->ctx.eip & 0xFFFF));
                KPCHAR(']');
                //kernelDebug();
                //while(1);
                return;
                break;
        }
    }
}

void doKernelPanic(void) {
    
    //initScreen();
    prints("Kernel panic:\n");
    prints("Unhandled interrupt #0x"); printHexByte(exeption_backup); prints(" triggered\n");
    kernelDebugWithProc(proc_backup);
    while(1); //Hang
}

void kernelEntry(void) {

    unsigned int kflags;
    unsigned short* stack;
    process* ret_p = p;

    //Backup the running context
    p->ctx.esp = _old_esp;
    p->ctx.cr3 = _old_cr3;
    p->ctx.eip = _old_eip;
    p->ctx.eflags = _old_eflags;
    p->ctx.eax = _old_eax;
    p->ctx.ecx = _old_ecx;
    p->ctx.edx = _old_edx;
    p->ctx.ebx = _old_ebx;
    p->ctx.ebp = _old_ebp;
    p->ctx.esi = _old_esi;
    p->ctx.edi = _old_edi;
    p->ctx.es = _old_es;
    p->ctx.cs = _old_cs;
    p->ctx.ss = _old_ss;
    p->ctx.ds = _old_ds;
    p->ctx.fs = _old_fs;
    p->ctx.gs = _old_gs;
    p->ctx.err = _old_err;

    //BIG ISSUE: Interrupts can put us here while we're still in the middle of a process,
    //so we need to make sure we do a check to see if we're in an interrupt first here
    //or, even better, figure out why interrupts put us here and keep that from happening
    if(p->flags & PF_V86)
        insPtr = (char*)(((((unsigned int)p->ctx.cs)&0xFFFF) << 4) + (((unsigned int)p->ctx.eip) &0xFFFF));
    else
        insPtr = (char*)p->ctx.eip;

    switch(_except_num) {

        case EX_GPF:
        //Switch to the V86 monitor if the thread was a V86 thread
            if(p->flags & PF_V86) {

                //In the case that needs_swap IS set, we know that
                //this is actually just a force-swap by the timer
                if(!needs_swap) {

                    V86Entry();
                }
            } else {

                //Otherwise, for now we just dump the system state and move on
                prints("(Non-V86)\n");
                kernelDebug();
                scans(5, fake);
            }
            break;

        case EX_SYSCALL:
            _syscall_number = p->ctx.eax;
            _syscall_param1 = p->ctx.ebx;
            _syscall_param2 = p->ctx.ecx;
            syscall_exec();
            break;

        case FORCE_ENTER:
            //We don't want to do anything here, this is just
            //so that we get an entry into and an exit from the kernel
            if(!_was_spurious) {

                DEBUG("\nA forceenter was requested\n");
                ret_p = c_timer_handler();

                //If there wasn't a process with an elapsed timer, stay in the
                //current process
                if(!ret_p)
                    ret_p = p;
            } else {

                c_spurious_handler();
            }

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

        //IRQs
        case 0xE1:
        case 0xE2:
        case 0xE3:
        case 0xE4:
        case 0xE5:
        case 0xE6:
        case 0xE7:
        case 0xE8:
        case 0xE9:
        case 0xEA:
        case 0xEB:
        case 0xEC:
        case 0xED:
        case 0xEE:
        case 0xEF:
            ret_p = irq_handle(_except_num);

            //If there isn't a process registered to this IRQ, we stay in the
            //current process
            if(!ret_p)
                ret_p = p;

            break;

        //In the default case, ensure that we're in text mode, clear the text mode screen and print the hang screen
        default:
            //Back up the current process structure, since we're going to be 
            //using enterTextMode to run a v86 interrupt, which will cause another
            //kernel exit and entry and therefore the info for the kernel panic 
            //is going to get overwritten
            proc_backup = p;
            exeption_backup = _except_num;
            
            //Turn off all hardware interrupts 
            disable_irq(0);
            disable_irq(1);
            disable_irq(2);
            disable_irq(3);
            disable_irq(4);
            disable_irq(5);
            disable_irq(6);
            disable_irq(7); //We would do all of them, but right now this only supports the first PIC
            enterTextMode(&doKernelPanic);
            break;
    }

    returnToProcess(ret_p);
}


void endProc(process* proc) {

    deleteProc(proc);

    if(proc == p)
        needs_swap = 1;

    returnToProcess(p);
}


void deleteProc(process* proc) {

    if(!(proc->flags & PF_V86)) {
        disable_page_range(0xB00000, proc->root_page);
        del_page_tree(proc->root_page);
        proc->root_page = (pageRange*)0;
    }

    if(proc->name)
        kfree(proc->name);

    proc->id = 0;
}


void clearContext(context* ctx) {

    int i;
    unsigned char* buf = (unsigned char*)ctx;

    for(i = 0; i < sizeof(context); i++)
        buf[i] = 0;
}


//ONLY TO BE USED IN VERY SPECIFIC SCENARIOS
void resetProcessCounter() {

    int i;

    nextProc = 1;
    swap_count = 0;

    for(i = 0; i < 255; i++)
        procTable[i].id = 0;

    p = (process*)0;
}


process* newProcess(char* name) {

    process* proc;
    int len = 0;
    int i;

    //fast-forward to the next free slot
    for(i = 0; procTable[i].id && (i < 256); i++);

    if(i == 256)
        return (process*)0x0;
    

    proc = &(procTable[i]);
    procPtr = (unsigned char)i;    
    proc->root_page = (pageRange*)0x0;
    proc->root_msg = (message*)0x0;
    proc->flags = 0;
    proc->cpu_pct = 0;
    proc->called_count = 0;
    proc->size = 0;

    //If the process was passed a name, we'll copy the
    //name into a fresh buffer to attach to the proc 
    if(name) {
        
        while(name[len++]);
        
        if(!(proc->name = (char*)kmalloc(len + 1)))
            return (process*)0x0;
            
        for(len = 0; name[len]; len++)
            proc->name[len] = name[len];
        
        proc->name[len] = 0;
    } else {
        
        proc->name = (char*)0;
    }

    //Finally, if we got this far, we give the proc a
    //PID, thereby marking this entry as taken 
    proc->id = nextProc++;

    return proc;
}


process* newUserProc(char* name) {

    process* newP;

    newP = newProcess(name);

    if(!newP)
        return newP;

    newP->base = 0xB00000;

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


process* newSuperProc(char* name) {

    process* newP;

    newP = newProcess(name);

    if(!newP)
        return newP;

    //Set the superproc bit
    newP->flags |= PF_SUPER;

    newP->base = 0xB00000;

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

    newP = newProcess((char*)0);

    if(!newP)
        return newP;

    //Set the v86 mode bit
    newP->flags |= PF_V86;

    newP->base = 0x80000;
    newP->size = 0x0;

    clearContext(&(newP->ctx));
    newP->ctx.esp = 0x1000;
    newP->ctx.ss = 0x9000;
    newP->ctx.ds = 0x8000;
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
    swap_count = 0;
}


//Append a new page to the end of the process's allocated virtual space
int request_new_page(process* proc) {

    int newSize;

    //prints("[kernel] Appending new page\n");
    newSize = append_page(proc->root_page);
    if(newSize) {

        //prints("[kernel] Success\n");
        proc->size += 0x1000;
        return 1;
    } else {

        //prints("[kernel] Fail (");
        //printHexDword(newSize);
        //prints(")\n");
        return 0;
    }
}


//Assumes code has already been loaded at 0x80000
unsigned int exec_loaded_v86(unsigned int app_size) {

    process* proc;

    if(!(proc = newV86Proc()))
        return 0;

    proc->size = app_size;
    return proc->id;
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

    DEBUG("Launching v86 process\n");

    return proc->id;
}


unsigned int exec_process(unsigned char* path, char isSuper) {

    FILE exeFile, exeFile2;
    process* proc;
    char* usrBase = (char*)0xB01000;
    int tmpVal, i;
    int pageCount;

    for(i = 0; path[i]; i++)
        pathBuf[i] = path[i];

    pathBuf[i] = 0;

    if(isSuper) {

        if(!(proc = newSuperProc(path)))
            return 0;
    } else {

        if(!(proc = newUserProc(path)))
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
    proc->size = i + 0x1000;

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

    DEBUG("Launching usermode process\n");

    return proc->id;
}

process* makeThread(process* parent, void* entry_point) {
    
    int i;
    process* ret_proc;
    pageRange* new_page;
    unsigned char* stack;
    unsigned char* tmp_stack;
    unsigned int* pageTable = (unsigned int*)PAGE_TABLE_ADDRESS;
    
    //Find a free proc entry 
    for(i = 0; i < 256 && procTable[i].id; i++);
    
    //Fail out if the process table is full
    if(i == 256)
        return (process*)0;
    
    ret_proc = &procTable[i];
    
    //Clone proc structures
    //Clone context 
    ret_proc->ctx.esp = parent->ctx.esp; //0xB00FFF;
    ret_proc->ctx.cr3 = parent->ctx.cr3;
    ret_proc->ctx.eip = parent->ctx.eip; //(unsigned int)entry_point;
    ret_proc->ctx.eflags = parent->ctx.eflags;
    ret_proc->ctx.eax = parent->ctx.eax;
    ret_proc->ctx.ecx = parent->ctx.ecx;
    ret_proc->ctx.edx = parent->ctx.edx;
    ret_proc->ctx.ebx = parent->ctx.ebx;
    ret_proc->ctx.ebp = parent->ctx.ebp;
    ret_proc->ctx.esi = parent->ctx.esi;
    ret_proc->ctx.edi = parent->ctx.edi;
    ret_proc->ctx.es = parent->ctx.es;
    ret_proc->ctx.cs = parent->ctx.cs;
    ret_proc->ctx.ss = parent->ctx.ss;
    ret_proc->ctx.ds = parent->ctx.ds;
    ret_proc->ctx.fs = parent->ctx.fs;
    ret_proc->ctx.gs = parent->ctx.gs;
    ret_proc->ctx.err = parent->ctx.err;
    ret_proc->ctx.vif = parent->ctx.vif;
    ret_proc->ctx.type = parent->ctx.type;
    
    //Clone process struct 
    ret_proc->id = nextProc++;
    ret_proc->root_msg = (message*)0;
    ret_proc->usr = parent->usr;
    ret_proc->base = parent->base;
    ret_proc->size = parent->size;
    ret_proc->flags = parent->flags;
    ret_proc->wait_pid = 0;
    ret_proc->wait_cmd = 0;
    ret_proc->called_count = 0;
    ret_proc->cpu_pct = 0;
    ret_proc->name = parent->name;
    
    //Clear status ailments (eg: the child thread won't be waiting for a message &c)
    ret_proc->flags &= ~(PF_WAITMSG | PF_WOKENMSG);
    
    //Split the first page (the stack page) from the first page range entry of the parent proc 
    if(parent->root_page->count > 1) {
        
        new_page = (pageRange*)kmalloc(sizeof(pageRange));
        
        //Clone the info for the first page into the new entry
        new_page->count = 1;
        new_page->base_page = parent->root_page->base_page;
        new_page->next = parent->root_page;
 
        //Update the old entry 
        parent->root_page->count--;
        parent->root_page->base_page++;
        
        //Replace the root page with the new root page 
        parent->root_page = new_page;               
    }
    
    //Create a page entry for the stack page of the new thread     
    ret_proc->root_page = (pageRange*)kmalloc(sizeof(pageRange));
    ret_proc->root_page->count = 1;
    
    //Try to allocate a new physical page, return failure if we can't 
    if(!(ret_proc->root_page->base_page = find_free_page())) {
        
        ret_proc->id = 0;
        kfree(ret_proc->root_page);
        return (process*)0;
    }
    
    //Mark the cloned stack page in-use
    pageTable[ret_proc->root_page->base_page] |= 0x00000800;
    ret_proc->root_page->next = parent->root_page->next;

    //Copy the old stack into the new stack to preserve sanity
    stack = (unsigned char*)0xB00000;
    tmp_stack = (unsigned char*)kmalloc(4096);
    
    //Get the old stack     
    for(i = 0; i < 4096; i++)
        tmp_stack[i] = stack[i];
        
    //Write to the new stack
    apply_page_range(ret_proc->base, ret_proc->root_page, ret_proc->flags & PF_SUPER);
    for(i = 0; i < 4096; i++)
        stack[i] = tmp_stack[i];
        
    //Clean up 
    apply_page_range(parent->base, parent->root_page, parent->flags & PF_SUPER);
    kfree((void*)tmp_stack);
 
    return ret_proc;
}
