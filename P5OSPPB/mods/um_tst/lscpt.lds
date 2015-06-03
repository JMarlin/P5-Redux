OUTPUT_FORMAT("elf32-i386")
ENTRY(_start)
phys = 0xB01000;

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
                *(.rodata)

        }
        .data : AT(phys + (data - startup))
        {
                data = .;
                *(.data)

        }
        .bss : AT(phys + (bss - startup))
        {
                bss = .;
                *(.bss)
                . = ALIGN(4096);
        }
        end = .;
}
