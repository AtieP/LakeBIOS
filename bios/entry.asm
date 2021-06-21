size: equ 0x10000

org 0xf0000

bits 16
bios_entry:
    cli

    ; Unlock BIOS so reads and writes will go to DRAM
    ; and not do DMI
    mov eax, 0x80000000 | 0x90
    mov dx, 0xcf8
    out dx, eax
    mov dx, 0xcfc + (0x90 & 3)
    in al, dx
    mov bl, al

    mov eax, 0x80000000 | 0x90
    mov dx, 0xcf8
    out dx, eax
    mov dx, 0xcfc + (0x90 & 3)
    mov al, bl
    out dx, al

    ; Go to protected mode
    mov ax, cs
    mov ds, ax

    lgdt [gdtr]

    mov eax, cr0
    or al, 1
    mov cr0, eax

    db 0x66
    db 0xea
    dd protected_mode
    dw 0x08

bits 32
protected_mode:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x2000
    jmp c_code
    hlt

bits 16
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
    dd gdt

times 1024 - ($ - $$) db 0x00
c_code:

incbin "cblob.bin"

times (size - 16) - ($ - $$) db 0x00

reset_vector:
    jmp 0xf000:0
    times 11 db 0xf4
