#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../../process/process.h"

#define PROC_TABLE_OFFSET 0x2029A0

typedef void (*sys_command)(process*, unsigned char*);
void rawDump(process* proc, unsigned char* ram);
void ctxDump(process* proc, unsigned char* ram);

#define CMD_COUNT 2

char* cmdWord[CMD_COUNT] = {
    "dump",
	"context"
};

sys_command cmdFunc[CMD_COUNT] = {
    (sys_command)&rawDump,
	(sys_command)&ctxDump
};

char cmdbuf[50];

void ctxDump(process* proc, unsigned char* ram) {
	
	printf("      esp: %08x  cr3: %08x  eip: %08x  eflags: %08x\n", proc->ctx.esp, proc->ctx.cr3, proc->ctx.eip, proc->ctx.eflags);
	printf("      eax: %08x  ecx: %08x  edx: %08x  ebx: %08x\n", proc->ctx.eax, proc->ctx.ecx, proc->ctx.edx, proc->ctx.ebx);
	printf("      ebp: %08x  esi: %08x  edi: %08x\n", proc->ctx.ebp, proc->ctx.esi, proc->ctx.edi);
	printf("      es: %04x  cs: %04x  ss: %04x  ds: %04x  fs: %04x  gs: %04x\n", proc->ctx.es, proc->ctx.cs, proc->ctx.ss, proc->ctx.ds, proc->ctx.fs, proc->ctx.gs);
	printf("      err: %08x  vif: %02x  type: %02x\n", proc->ctx.err, proc->ctx.vif, proc->ctx.type);
}

void rawDump(process* proc, unsigned char* ram) {
	
	printf("      id: %d\n", proc->id);
	printf("      root_page: %08x\n", (unsigned int)proc->root_page);
	printf("      root_msg: %08x\n", (unsigned int)proc->root_msg);
	printf("      usr: %08x\n", (unsigned int)proc->usr);
	printf("      base: %08x\n", proc->base);
	printf("      size: %08x\n", proc->size);
	printf("      flags: %08x\n", proc->flags);
	printf("      wait_pid: %08x\n", proc->wait_pid);
	printf("      wait_cmd: %08x\n", proc->wait_cmd);
	printf("      called_count: %d\n", proc->called_count);
	printf("      cpu_pct: %d\n", proc->cpu_pct);
}

int main(int argc, char* argv[]) {
	
	FILE* ramfile;
	process *procTable, *cur_proc;
	unsigned char* rawbuf;
	int i, pid;
	
	if(argc != 2) {
		
		printf("Usage: %s ramdump.file\n", argv[0]);
		return 0;
	}
	
	if(!(rawbuf = (unsigned char*)malloc(0xA00000))) {
		
		printf("Couldn't allocate memory space for the memory image\n");
		return 0;
	}
	
	if(!(ramfile = fopen(argv[1], "rb"))) {
		
		printf("Couldn't open file %s\n", argv[1]);
		free(rawbuf);
		return 0;
	}
		
	if(fread((void*)rawbuf, 0x100000, 10, ramfile) != 10) {
		
		printf("Failed reading image into ram\n");
		fclose(ramfile);
		free(procTable);
		return 0;
	}
	
	fclose(ramfile);
	
	procTable = (process*)(&rawbuf[PROC_TABLE_OFFSET]);
	
	while(1) {
	
		//Prompt for process number
		printf("Process table:\n");
		
		for(i = 0; i < 256; i++) {
		
			if(procTable[i].id) printf("    [%d]: id: %d\n", i, procTable[i].id);
		}

		while(1) {
					
			printf("Which pid do you want to explore?: ");
			scanf("%u", &pid);
		
			for(i = 0; i < 256; i++) {
				
				if(procTable[i].id == pid) 
					break;	
			}
		
			if(i == 256) 
				printf("No such pid\n");
			else 
				break;
		}
	
		cur_proc = &procTable[i];
	
		while(1) {
		
			printf("Do what with pid %u?: ", pid);
			scanf("%49s", cmdbuf);
			
			if(!strcmp("back", cmdbuf))
				break;
				
			if(!strcmp("quit", cmdbuf)) {
				
				free(rawbuf);
				return 0;
			}
			
			for(i = 0; i < CMD_COUNT; i++) {
		
				if(!strcmp(cmdWord[i], cmdbuf)) {
		
					cmdFunc[i](cur_proc, rawbuf);
					break;
				}
			}
		
			if(i == CMD_COUNT) {
		
				printf("Unknown command ");
				printf(cmdbuf);
				printf("\n");
			} 
		}
	}
}