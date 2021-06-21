OUTPUT_FORMAT(binary)

SECTIONS {
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
}
