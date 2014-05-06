#include "../ASCII_IO/ASCII_O.H"
#include "COFF.H"
#define DEBUG

//The following function recieves the base address of
//a COFF image which is loaded into memory and uses
//the relocation table found within to apply the necissary
//transformations to the embedded code sections so that
//later operations can jump into that code and operate
//as one would expect.

int strcmp(char *stra, char *strb) {

        int i;

        for(i = 0; stra[i] && strb[i] && stra[i] == strb[i]; i++);
        return stra[i] == strb[i];

}

void* coff_resolveSymbol(char* symbolName, void* coffBaseAddr) {

#ifdef DEBUG
        prints("Symbols:\n");
#endif

        int i, j;
        unsigned char shortName[9], *curSymbolName;
        coff_header *header;
        symbol *currentSymbol;
        char* stringTable;
        section_header* sectionHeader;

        header = (coff_header*)coffBaseAddr;
        stringTable = (char*)(coffBaseAddr + header->symbol_table_offset + (header->symbol_count * 18));

        for(i = 0; i < header->symbol_count; i++){

                currentSymbol = (symbol*)(coffBaseAddr + header->symbol_table_offset + (18*i));

                //Get the name of the current symbol
                if(currentSymbol->name_zeros){

                        for(j = 0; j < 8; j++){
                                shortName[j] = currentSymbol->name[j];
                        }
                        shortName[j] = 0;
                        curSymbolName = shortName;

                }else{

                        curSymbolName = (char*)(stringTable + currentSymbol->name_offset);
                        
                }

/*
#ifdef DEBUG
                prints(curSymbolName);
                prints(" -> ");
                sectionHeader = (section_header*)(coffBaseAddr + 20 + header->optional_header_size + ((currentSymbol->section_number - 1)*40));
                printHexLong((long)(coffBaseAddr + sectionHeader->data_offset + currentSymbol->value));
                prints("\n");
#endif
*/

                if(strcmp(curSymbolName, symbolName)){
                        break;
                }

                //Skip any aux symbols
                i += currentSymbol->aux_count;

        }
        
        if(i < header->symbol_count) {
                sectionHeader = (section_header*)(coffBaseAddr + 20 + header->optional_header_size + ((currentSymbol->section_number - 1)*40));
                return (void*)(coffBaseAddr + sectionHeader->data_offset + currentSymbol->value);
        }else{
                return 0;
        }
        
}

void coff_applyRelocations(void* coffBaseAddr){

       symbol* relocationSymbol;
       relocation_entry* relocationEntry;
       int i, j;
       long *relocationRelPtr;
       unsigned long *relocationPtr, symbolAddress;
       section_header *sectionHeader, *symbolSection;

       //First, we will reference the start of the COFF
       //image as a coff header structure for further operations
       coff_header *header = (coff_header*)coffBaseAddr;

#ifdef DEBUG
       prints("Header for COFF image at location 0x");
       printHexLong((unsigned long)coffBaseAddr);
       prints("\n");
       prints("        magic: 0x");
       printHexShort(header->magic);
       prints("\n");
       prints("        section_count: 0x");
       printHexShort(header->section_count);
       prints("\n");
       prints("        timestamp: 0x");
       printHexLong(header->timestamp);
       prints("\n");
       prints("        symbol_table_offset: 0x");
       printHexLong(header->symbol_table_offset);
       prints("\n");
       prints("        symbol_count: 0x");
       printHexLong(header->symbol_count);
       prints("\n");
       prints("        optional_header_size: 0x");
       printHexShort(header->optional_header_size);
       prints("\n");
       prints("        flags: 0x");
       printHexShort(header->flags);
       prints("\n");
#endif

       /*
       1) Resolve relocations:
                For each section:
                        Get section relocation entries start
                        For each relocation entry:
                                Find the symbol entry referred to
                                Calculate the position of the symbol in the
                                associated section
                                Add the proper offset to the existing value
       */

       for(i = 0; i < header->section_count; i++) {
                sectionHeader = (section_header*)(coffBaseAddr + 20 + header->optional_header_size + (i*40));
                for(j = 0; j < sectionHeader->relocation_entries_count; j++) {
                        relocationEntry = (relocation_entry*)(coffBaseAddr + sectionHeader->relocation_entries_offset + (j*10));
                        relocationSymbol = (symbol*)(coffBaseAddr + header->symbol_table_offset + ((relocationEntry->symbol_index)*18));
                        symbolSection = (section_header*)(coffBaseAddr + 20 + header->optional_header_size + ((relocationSymbol->section_number - 1)*40));
                        relocationPtr = (unsigned long*)(coffBaseAddr + sectionHeader->data_offset + relocationEntry->virtual_address);
                        relocationRelPtr = (long*)relocationPtr;
                        symbolAddress = (unsigned long)(coffBaseAddr + symbolSection->data_offset + relocationSymbol->value);

#ifdef DEBUG
                        prints("[0x");
                        printHexLong((unsigned long)relocationPtr);
                        prints("]: 0x");
                        printHexLong(relocationPtr[0]);
                        prints(" -> 0x");
#endif
                        
                        if(relocationEntry->type == RELOC_ADDR32){
                                relocationPtr[0] += symbolAddress;
                        }
                        
                        if(relocationEntry->type == RELOC_REL32){
                               //relocationRelPtr[0] += symbolAddress;
                               //relocationRelPtr[0] -= (long)(coffBaseAddr + symbolSection->data_offset);
                               //relocationRelPtr[0] += (long)sectionHeader->data_offset;
                        }

#ifdef DEBUG
                        printHexLong(relocationPtr[0]);
                        prints("\n");
#endif

                        
                }
                
       }

}
