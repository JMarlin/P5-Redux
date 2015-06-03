OUTPUT_FORMAT("elf32-i386")
ENTRY(_start)
phys = 0x100000;

__DYNAMIC = 0;

SECTIONS
{
        .init phys : AT(phys) {
                startup = .;
                *(.init)
        }

        .text : AT(phys + (code - startup)) {
                code = .;
                *(.text)
        }
        .data : AT(phys + (data - startup))
        {
                data = .;
                *(.data)
                *(.rodata)
                *(.bss)
        }
        .bss : AT(phys + (bss - startup))
        {
                bss = .;
                *(.bss)
        }
        end = .;
}
