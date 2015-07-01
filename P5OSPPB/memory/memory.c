#include "memory.h"
#include "../ascii_io/ascii_o.h"
#include "../core/global.h"
#include "../core/syscall.h"

#define MAX_MMAPS 30


extern long pkgoffset;
unsigned long maxRAM = 0x006FFFFF;
memblock rootBlock;
void (*init_done)(void);
memzone m_map[MAX_MMAPS];
static char mmap_index = 0;

void testRAM() {

    int i = 0xB00000;
    unsigned char prev;
    unsigned char* sysram = (unsigned char*)0;

    while(1) {
        prev = sysram[i];
        sysram[i] = ~prev;

        if(sysram[i] == prev)
            break;

        sysram[i] = prev;
        printHexDword(i);
        prints("\n");
        i++;
    }

    maxRAM = i;
}


void printChain() {

    memblock* nextBlock = &rootBlock;

    prints("Current allocated memory:\n");

    while(1) {
        prints("   Start: 0x");
        printHexDword((unsigned long)nextBlock->base);
        prints(", Size: 0x");
        printHexDword(nextBlock->size);
        prints("\n");

        if(nextBlock->next)
            nextBlock = nextBlock->next;
        else
            break;
    }
}


void finish_mem_config(void);
void get_next_memzone(unsigned int ebx, unsigned int ecx, unsigned int edx);


void init_memory(void (*cb)(void)) {

    init_done = cb;

    get_next_memzone(0, 0, 0);
}


void get_next_memzone(unsigned int ebx, unsigned int ecx, unsigned int edx) {

    //Only one v86 proc can exist at a time and
    //they're all loaded into 0x80000
    char* usrCode = (char*)0x80000;
    memzone* memz_buf = (memzone*)0x81000; //Where the v86 code will put the entry

    //We'll use this to figure out if we're done or not
    static char in_list = 0;

    set_call_zero_cb(&get_next_memzone); //Make this shit recursive

    //If we're in the list, we have a valid entry and should store it
    if(in_list) {

        if(ecx != 0x534D4150) {

            finish_mem_config();
        }

        m_map[mmap_index].base = memz_buf->base;
        m_map[mmap_index].length = memz_buf->length;
        m_map[mmap_index].type = memz_buf->type;
        mmap_index++;

        //Exit early so we don't overflow the array
        if(mmap_index == MAX_MMAPS) {

            finish_mem_config();
        }
    }

    //We're either at the beginning of the list or the end
    if(!ebx) {

        if(in_list) {

            finish_mem_config();
        } else {

            in_list = 1;
        }
    }

    //Write our shite to low ram
    //Do INT 0x15
    usrCode[0]  = 0x8C; // -|
    usrCode[1]  = 0xC8; // -\_mov ax, cs
    usrCode[2]  = 0x8E; // -|
    usrCode[3]  = 0xC0; // -\_mov es, ax
    usrCode[4]  = 0xBF; // -|
    usrCode[5]  = 0x00; //  |
    usrCode[6]  = 0x10; // -\_mov di, 0x1000
    usrCode[7] = 0x66; // --------------------------------|
    usrCode[8] = 0xBB; //                                 |
    usrCode[9] = (unsigned char)((ebx >> 24) & 0xFF); //  |
    usrCode[10] = (unsigned char)((ebx >> 16) & 0xFF); // |
    usrCode[11] = (unsigned char)((ebx >> 8) & 0xFF);  // |
    usrCode[12] = (unsigned char)(ebx & 0xFF);         //-\_mov ebx, <the actual ebx arg>
    usrCode[13] = 0x66; // -|
    usrCode[14] = 0xBA; //  |
    usrCode[15] = 0x50; //  |
    usrCode[16] = 0x41; //  |
    usrCode[17] = 0x4D; //  |
    usrCode[18] = 0x53; // -\_mov edx, 0x534D4150
    usrCode[19] = 0x66; // -|
    usrCode[20] = 0xB9; //  |
    usrCode[21] = 0x00; //  |
    usrCode[22] = 0x00; //  |
    usrCode[23] = 0x00; //  |
    usrCode[24] = 0x24; // -\_mov ecx, 0x00000024
    usrCode[25] = 0x66; // -|
    usrCode[26] = 0xB8; //  |
    usrCode[27] = 0x20; //  |
    usrCode[28] = 0xE8; //  |
    usrCode[29] = 0x00; //  |
    usrCode[30] = 0x00; // -\_mov eax, 0x0000E820
    usrCode[31] = 0xCD; // -|
    usrCode[32] = 0x15; // -\_int 0xff
    //Do INT 0xFF #0, return to kernel init
    usrCode[33] = 0x66; // -|
    usrCode[34] = 0x89; //  |
    usrCode[35] = 0xC1; // -\_mov ecx, eax
    usrCode[36] = 0x66; // -|
    usrCode[37] = 0x31; //  |
    usrCode[38] = 0xC0; // -\_xor eax, eax
    usrCode[39] = 0xCD; // -|
    usrCode[40] = 0xFF; // -\_int 0xff

    enterProc(exec_loaded_v86(100));
}


void finish_mem_config() {

    void* ram_a;
    void* ram_b;
    int i;
    unsigned long long end;

    for(i = 0; i < mmap_index; i++) {

        end = m_map[i].base + m_map[i].length;

        prints("0x");
        printHexDword((unsigned int)((m_map[i].base & 0xFFFFFFFF00000000) >> 32));
        printHexDword((unsigned int)(m_map[i].base & 0xFFFFFFFF));
        prints(" to 0x");
        printHexDword((unsigned int)((end & 0xFFFFFFFF00000000) >> 32));
        printHexDword((unsigned int)(end & 0xFFFFFFFF));
        prints(" is ");

        switch(m_map[i].type) {

            case 1:
                prints("free space\n");
            break;

            case 3:
                prints("ACPI reclaimable\n");
            break;

            case 4:
                prints("ACPI NVS\n");
            break;

            case 5:
                prints("bad memory\n");
            break;

            default:
                prints("reserved\n");
            break;
        }
        //printHexDword(ebx);
        //pchar('\n');
    }

    //Reset the state changes made by creating all of those v86 procs
    resetProcessCounter();

    //Start of kernel
    rootBlock.base = (void*)0x100000;
    rootBlock.size = 0x200000;
    rootBlock.next = (memblock*)0;


    //testRAM();
    prints("Top of RAM: 0x");
    printHexDword(maxRAM);
    prints("\nAllocating 1k of RAM to ram_a.\n");
    ram_a = kmalloc(1024);

    if(ram_a == (void*)0) {
        prints("Allocation failed.\n");
        return;
    }

    //printChain();
    prints("Allocating 1k of RAM to ram_b.\n");
    ram_b = kmalloc(1024);

    if(ram_b == (void*)0) {
        prints("Allocation failed.\n");
        return;
    }

    //printChain();
    prints("Freeing ram_a.\n");
    ram_a = kfree(ram_a);

    if(ram_a != (void*)0) {
        prints("Free failed.\n");
        return;
    }

    //printChain();
    prints("Freeing ram_b.\n");
    ram_b = kfree(ram_b);

    if(ram_b != (void*)0) {
        prints("Free failed.\n");
        return;
    }

    //printChain();
    init_done(); //Return to kernel startup
}


memblock* getMBTail() {

    memblock* nextBlock = &rootBlock;

    while(nextBlock->next)
        nextBlock = nextBlock->next;

    return nextBlock;
}

memblock* nextMBByAddress(void* baseAddr) {

    memblock* nextBlock = &rootBlock;
    memblock* returnBlock;
    void* nextBase = (void*)maxRAM;
    int blockFound = 0;

    while(1) {
        //DEBUG("Checking address "); //DEBUG_HD((unsigned long)baseAddr);
        //DEBUG(" against block "); //DEBUG_HD((unsigned long)nextBlock->base);
        //DEBUG("-"); //DEBUG_HD((unsigned long)nextBlock->base + nextBlock->size);
        //DEBUG("...");

        if(nextBlock->base >= baseAddr && nextBlock->base < nextBase) {
            returnBlock = nextBlock;
            nextBase = returnBlock->base;
            blockFound = 1;
            //DEBUG("match.\n");
        } else {
            //DEBUG("no match.\n");
        }

        if(nextBlock->next)
            nextBlock = nextBlock->next;
        else
            break;
    }

    if(blockFound)
        return returnBlock;
    else
        return (memblock*)0;
}


int MBCollision(void* base, unsigned long size) {

    unsigned int ibase = (unsigned int)base;
    unsigned int isize = (unsigned int)size;
    unsigned int nbase, nsize;
    memblock* nextBlock = &rootBlock;

    while(1) {
        nbase = (unsigned int)nextBlock->base;
        nsize = (unsigned int)nextBlock->size;
        if((ibase >= nbase && ibase < nbase + nsize) ||
           (ibase + isize >= nbase && ibase + isize < nbase + nsize) ||
           (ibase <= nbase && ibase + isize > nbase + nsize) ||
           (ibase + isize > maxRAM))
                return 1;

        if(nextBlock->next)
            nextBlock = nextBlock->next;
        else
            return 0;
    }
}


void* kmalloc(unsigned int size) {

    memblock* tailBlock = getMBTail();
    memblock* nextBlock;
    memblock* newBlock;

    //This needs to be fixed when we start playing with
    //actual packages as allocations will clobber the packages
    void* rambase = (void*)(0x00300000); //User RAM starts at 3MB

    while(1) {

        if(!MBCollision(rambase, size + sizeof(memblock)))
            break;

        if((nextBlock = nextMBByAddress(rambase)) == (memblock*)0)
            return (void*)0;

        rambase = nextBlock->base + nextBlock->size;
    }

    tailBlock->next = (memblock*)rambase;
    newBlock = tailBlock->next;
    newBlock->next = (memblock*)0;
    newBlock->base = rambase;
    newBlock->size = size + sizeof(memblock);
    return rambase + sizeof(memblock);
}


void* kfree(void* base) {

    memblock* nextBlock = &rootBlock;
    memblock* prevBlock = &rootBlock;
    void* realBase = base - sizeof(memblock);

    //DEBUG("Free base: "); //DEBUG_HD((unsigned long)base);
    //DEBUG("\nReal base: "); //DEBUG_HD((unsigned long)realBase);
    //DEBUG("\n");

    while(1) {
        //DEBUG("   Matches ");
        //DEBUG_HD((unsigned long)nextBlock->base);
        //DEBUG("?...");

        if(nextBlock->base == realBase) {
            //DEBUG("yes\n");
            prevBlock->next = nextBlock->next;
            return (void*)0;
        } else {
            //DEBUG("no\n");
        }

        if(nextBlock->next) {
            prevBlock = nextBlock;
            nextBlock = nextBlock->next;
        } else {
            return base;
        }
    }
}
