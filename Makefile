AS = nasm
ASFLAT := -f bin

BIOS = atiebios.bin

all: $(BIOS)

$(BIOS): bios/entry.asm
	$(AS) $(ASFLAT) $< -o $@

run:
	qemu-system-x86_64 -M q35 -no-reboot -bios $(BIOS)

clean:
	rm $(BIOS)
