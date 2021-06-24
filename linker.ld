OUTPUT_FORMAT(binary)

SECTIONS {
    bios_code_start = 0xf0000;
    . = 0xf0400;
    smm_entry_code_start = .;
    . = 0xf0800;
    smm_entry_code_end = .;
    smm_main_code_start = .;
    .smm_main_code : {
        KEEP(*(.smm_main_code))
    }
    . = 0xf1800;
    smm_main_code_end = .;

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
