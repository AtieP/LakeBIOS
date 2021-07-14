OUTPUT_FORMAT(binary)

SECTIONS {
    smm_trampoline_start = 0xfe000;
    smm_trampoline_end = 0xff000;

    . = 0xe0000;
    bios_raw_start = .;
    bios_data_start = .;

    .smm_stack : {
        *(.smm_stack*)
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

    . = 0xf0000;
    bios_data_end = .;
    bios_code_start = .;

    .smm_entry : {
        KEEP(*(.smm_entry*))
    }

    . = 0xf1000;
    .text : {
        KEEP(*(.bios_init*))
        *(.text*)
    }

    bios_code_end = .;
    bios_raw_end = .;
}
