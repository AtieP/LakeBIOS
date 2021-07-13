bios_size: equ 0x20000
bios_init: equ 0xf1000
smm_entry: equ 0xf0000


org 0xe0000

incbin "blob.bin"

times (bios_size - (4096 * 3)) - ($ - $$) db 0x00

bits 16

real_mode_handlers:

%macro real_mode_handler 1
align 16
    ; This is insanity, but EAX needs to be preserved somehow :shrug:
    mov cr2, eax
    mov al, i
    out 0xb3, al
    mov al, 0x10
    out 0xb2, al
    mov eax, cr2
    iret
%endmacro

%assign i 0
%rep 256
real_mode_handler i
%assign i i + 1
%endrep

smm_trampoline:
    cld

    mov ax, 0xf000
    mov ds, ax

    lgdt [early_init.early_gdtr]

    mov eax, cr0
    or al, 1
    mov cr0, eax

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0xa7000

    jmp dword 0x08:smm_entry

times 4096 - ($ - smm_trampoline) db 0x00

early_init:
    ; Some people like to jump to the reset vector. Mitigate against
    ; possible corruptions
    cli
    cld

    ; KVM doesn't like the GDT being in read only memory.
    ; Move it somewhere else in RAM, it doesn't matter where
    ; exactly really since during POST the BIOS will load its
    ; own GDT
    mov ax, cs
    mov ds, ax
    xor ax, ax
    mov es, ax

    mov si, .early_gdt
    mov di, 0x400
    mov cx, .early_gdt.end - .early_gdt
    rep movsb

    lgdt [.early_gdtr]

    mov eax, cr0
    or al, 1
    mov cr0, eax

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x2000

    jmp dword 0x08:bios_init

.early_gdt:
    ; null
    dq 0

    ; code
    dw 0xffff
    dw 0x0000
    db 0x00
    db 10011010b
    db 11001111b
    db 0x00

    ; data
    dw 0xffff
    dw 0x0000
    db 0x00
    db 10010010b
    db 11001111b
    db 0x00

.early_gdt.end:

.early_gdtr:
    dw .early_gdt.end - .early_gdt - 1
    dd 0x400

times (bios_size - 16) - ($ - $$) db 0x00

reset_vector:
    jmp 0xf000:early_init
    times 11 db 0
