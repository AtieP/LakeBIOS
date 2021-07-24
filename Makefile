CFILES := $(shell find src/ -type f -name '*.c')
LDFILE := linker.ld
OBJS := $(ASFILES:.asm=.o) $(CFILES:.c=.o)
BIOS = lakebios.bin

CC = gcc
CFLAGS = -m32 -mno-sse -mno-sse2 -mno-mmx -mno-3dnow -mno-80387 -nostdlib -ffreestanding -fno-pic -fno-stack-protector -std=gnu99 -O2 -Wall -Wextra -Isrc/ -lgcc -static -c

AS = nasm
ASFLAGS := -f bin

LD = ld
LDFLAGS := -T$(LDFILE) -nostdlib -m elf_i386 -static -n

QEMU = qemu-system-x86_64
QEMUFLAGS := -no-reboot -bios $(BIOS) -debugcon stdio -device ramfb -fw_cfg "opt/wallpaper",file=wallpaper.bmp

all: $(BIOS)

$(BIOS): $(OBJS) src/entry.asm
	$(LD) $(LDFLAGS) $(OBJS) -o blob.bin
	$(AS) $(ASFLAGS) src/entry.asm -o $@

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

run-q35:
	qemu-system-x86_64 -M q35 $(QEMUFLAGS)

run-q35-kvm:
	qemu-system-x86_64 -M q35 $(QEMUFLAGS) -enable-kvm

run-i440fx:
	qemu-system-x86_64 -M pc $(QEMUFLAGS)

run-i440fx-kvm:
	qemu-system-x86_64 -M pc $(QEMUFLAGS) -enable-kvm

clean:
	rm $(OBJS) blob.bin $(BIOS)
