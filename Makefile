CFILES := $(shell find src/ -type f -name '*.c')
LDFILE := linker.ld
OBJS := $(ASFILES:.asm=.o) $(CFILES:.c=.o)
BIOS = lakebios.bin

CC = gcc
CFLAGS = -m32 -mno-sse -mno-sse2 -mno-mmx -mno-3dnow -mno-80387 -nostdlib -ffreestanding -fno-pic -fno-stack-protector -std=c11 -pedantic -O2 -Wall -Wextra -Isrc/ -lgcc -static -c

AS = nasm
ASFLAGS := -f bin

LD = ld
LDFLAGS := -T$(LDFILE) -nostdlib -m elf_i386 -static -n

QEMU = qemu-system-x86_64
QEMUFLAGS := -bios $(BIOS) -debugcon stdio

HEADERDEPDS = $(OBJS:%.o=%.d)

all: $(BIOS)

$(BIOS): $(OBJS) src/entry.asm
	$(LD) $(LDFLAGS) $(OBJS) -o blob.bin
	$(AS) $(ASFLAGS) src/entry.asm -o $@

-include $(HEADERDEPDS)

%.o: %.c
	$(CC) $(CFLAGS) -MMD $< -o $@

run-q35:
	qemu-system-x86_64 -M q35 $(QEMUFLAGS)

run-q35-kvm:
	qemu-system-x86_64 -M q35 $(QEMUFLAGS) -enable-kvm

run-i440fx:
	qemu-system-x86_64 -M pc $(QEMUFLAGS)

run-i440fx-kvm:
	qemu-system-x86_64 -M pc $(QEMUFLAGS) -enable-kvm

clean:
	rm -f $(OBJS) blob.bin $(BIOS) $(HEADERDEPDS)

graph:
	cflow2dot -i $(CFILES) -f dot --source bios_main
	cat *.dot | gvpack -o svg.dot
	dot -Tsvg svg.dot > graph.svg
	rm *.dot
