bios_size: equ 0x10000
bios_main: equ 0xf1800
smm_main:  equ 0xf0800

gdt_addr: equ 0x400
stack:    equ 0x2000

org 0xf0000

bits 16
bios_entry:
    cli

    ; Temporary workaround
    mov ax, cs
    mov ds, ax
    xor ax, ax
    mov es, ax
    mov si, gdt
    mov di, gdt_addr
    mov cx, gdt.end - gdt
    rep movsb

    ; Go to protected mode
    lgdt [gdtr]

    mov eax, cr0
    or al, 1
    mov cr0, eax

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, stack

    jmp dword 0x08:bios_main

bits 16
; Initial GDT for loading the BIOS, the BIOS will load its own GDT later
gdt:
    dq 0

.code:
    dw 0xffff
    dw 0x0000
    db 0x00
    db 10011010b
    db 11001111b
    db 0x00

.data:
    dw 0xffff
    dw 0x0000
    db 0x00
    db 10010010b
    db 11001111b
    db 0x00

.end:

gdtr:
    dw gdt.end - gdt - 1
    dd gdt_addr

times 1024 - ($ - $$) db 0x00
smm_entry_code:
    mov ax, 0xf000
    mov ds, ax

    lgdt [gdtr]

    mov eax, cr0
    or al, 1
    mov cr0, eax

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    jmp dword 0x08:smm_main

times 1024 - ($ - smm_entry_code) db 0x00

incbin "cblob.bin"

times (bios_size - 16) - ($ - $$) db 0x00

reset_vector:
    jmp 0xf000:0
    times 11 db 0xf4
