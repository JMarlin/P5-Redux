//coff header flag values
#define F_LNNO 0x4 //There is no line number data in this image
#define F_AR32WR 0x100 //Image is 32-bit, little endian

//section header flag values
#define STYP_TEXT 0x20 //Section is executable code
#define STYP_DATA 0x40 //Section is initialized data
#define STYP_BSS 0x80 //Section is uninitialized data, not included in image
#define IMAGE_SCN_LNK_INFO 0x200 //Comments or other info

//relocation entry type values
#define RELOC_ADDR32 0x6 //An absolute 32-bit reference
#define RELOC_REL32 0x14 //A relative 32-bit reference

//symbol section number alternate values
#define N_DEBUG -2 //The symbol is a debugging symbol

//symbol type values
#define T_NULL 0x0 //Not a real symbol

//symbol storage class values
#define C_FILE 0x67 //The symbol is the name of the source file
#define C_STAT 0x3 //A static/private symbol

//coff structures
typedef struct SCTcoff_header {
        unsigned short magic;
        unsigned short section_count;
        unsigned long timestamp;
        unsigned long symbol_table_offset;
        unsigned long symbol_count;
        unsigned short optional_header_size;
        unsigned short flags;
} coff_header;

typedef struct SCTsection_header {
        union {
                unsigned char name[8];
                struct {
                        unsigned long name_zeros;
                        unsigned long name_string_offset;
                };
        };
        unsigned long physical_address;
        unsigned long virtual_address;
        unsigned long size;
        unsigned long data_offset;
        unsigned long relocation_entries_offset;
        unsigned long line_number_entries_offset;
        unsigned short relocation_entries_count;
        unsigned short line_number_entries_count;
        unsigned long flags;
} section_header;

typedef struct SCTrelocation_entry {
        unsigned long virtual_address;
        unsigned long symbol_index;
        unsigned short type;
} relocation_entry;

typedef struct SCTsymbol {
        union {
                unsigned char name[8];
                struct {
                        unsigned long name_zeros;
                        unsigned long name_offset;
                };
        };
        unsigned long value;
        short section_number;
        unsigned short type;
        unsigned char storage_class;
        unsigned char aux_count;
} symbol;

//coff access functions
void coff_applyRelocations(void* coffBaseAddr);
