OUTPUT_FORMAT(binary)

SECTIONS {
    bios_code_start = 0xf0000;
    . = 0xf0400;

    .text : {
        KEEP(*(.c_init*))
        *(.text*)
    }

    .data : {
        *(.data*)
    }

    .rodata : {
        *(.rodata*)
    }

    .bss : {
        *(.bss*)
    }

    . = 0xffff0;
    bios_code_end = 0x100000;
}
