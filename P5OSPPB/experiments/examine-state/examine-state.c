#include <stdlib.h>
#include "../../process/process.h"

#define PROC_TABLE_OFFSET 0x2029A0

int main(int argc, char* argv[]) {
	
	FILE* ramfile;
	process* procTable;
	unsigned int procTableSize = sizeof(process)*256;
	int i;
	
	if(argc != 2) {
		
		prints("Usage: %s ramdump.file\n", argv[0]);
		return 0;
	}
	
	if(!(procTable = (process*)malloc(sizeof(process)*256))) {
		
		prints("Couldn't allocate memory space for the proc table\n");
		return 0;
	}
	
	if(!(ramfile = fopen(argv[1], "rb"))) {
		
		prints("Couldn't open file %s\n", argv[1]);
		free(procTable);
		return 0;
	}
	
	if(fseek(ramfile, PROC_TABLE_OFFSET, SEEK_SET)) {
		
		prints("Failed seeking to proc table offset\n");
		fclose(ramfile);
		free(procTable);
		return 0;
	}
	
	if(fread((void*)procTable, sizeof(process), 256, ramfile) != 256) {
		
		prints("Failed reading proc table from image\n");
		fclose(ramfile);
		free(procTable);
		return 0;
	}
	
	fclose(ramfile);
	
	for(i = 0; i < 256; i++) {
		
		if(procTable[i].id) {
			
			prints("[%d]: %d\n", i, procTable[i].id);
		}
	}
	
	free(procTable);
}