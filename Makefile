CFILES := $(shell find . -type f -name '*.c')
LDFILE := linker.ld
OBJS := $(ASFILES:.asm=.o) $(CFILES:.c=.o)
BIOS = atiebios.bin

CC = gcc
CFLAGS = -m32 -mno-sse -mno-sse2 -mno-mmx -mno-3dnow -mno-80387 -nostdlib -ffreestanding -fno-pic -fno-stack-protector -std=gnu99 -O2 -Wall -Wextra -Ibios/ -lgcc -static -c

AS = nasm
ASFLAGS := -f bin

LD = ld
LDFLAGS := -T$(LDFILE) -nostdlib -m elf_i386 -static -n

all: $(BIOS)

$(BIOS): $(OBJS) bios/entry.asm
	$(LD) $(LDFLAGS) $(OBJS) -o cblob.bin
	$(AS) $(ASFLAGS) bios/entry.asm -o $@

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

run:
	qemu-system-x86_64 -M q35 -no-reboot -bios $(BIOS) -debugcon stdio -vga std -device ramfb

clean:
	rm $(OBJS) cblob.bin $(BIOS)
