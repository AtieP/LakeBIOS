# Possible targets:
# QEMU:
# * qemu-i440fx-piix
# * qemu-q35-ich9
# * qemu-hybrid (both qemu-i440fx-piix and qemu-q35-ich9)

CFILES := $(shell find src/ -type f -name '*.c' -not -path 'src/motherboard/*')
CC = gcc
CFLAGS = -m32 -mno-sse -mno-sse2 -mno-mmx -mno-3dnow -mno-80387 -nostdlib -ffreestanding -fno-pic -fno-stack-protector -std=c11 -pedantic -O2 -Wall -Wextra -Isrc/ -lgcc -static -c

ifdef TARGET
	ifeq ($(TARGET),qemu-i440fx-piix)
		CFILES += $(shell find src/motherboard/qemu/ -maxdepth 1 -type f -name '*.c')
		CFILES += $(shell find src/motherboard/qemu/i440fx/ src/motherboard/qemu/piix -type f -name '*.c')
		CFLAGS += -D QEMU_I440FX_PIIX
	else ifeq ($(TARGET),qemu-q35-ich9)
		CFILES += $(shell find src/motherboard/qemu/ -maxdepth 1 -type f -name '*.c')
		CFILES += $(shell find src/motherboard/qemu/q35/ src/motherboard/qemu/ich9 -type f -name '*.c')
		CFLAGS += -D QEMU_Q35_ICH9
	else ifeq ($(TARGET),qemu-hybrid)
		CFILES += $(shell find src/motherboard/qemu/ -type f -name '*.c')
		CFLAGS += -D QEMU_I440FX_PIIX -D QEMU_Q35_ICH9
	else
		$(error Unknown TARGET)
	endif
endif

LDFILE := linker.ld
OBJS := $(ASFILES:.asm=.o) $(CFILES:.c=.o)
BIOS = lakebios.bin

AS = nasm
ASFLAGS := -f bin

LD = ld
LDFLAGS := -T$(LDFILE) -nostdlib -m elf_i386 -static -n

QEMU = qemu-system-x86_64
QEMUFLAGS := -bios $(BIOS) -debugcon stdio

HEADERDEPDS = $(OBJS:%.o=%.d)

.PHONY: all clean

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
	$(eval CFILES += $(shell find src/motherboard -type f -name '*.c'))
	$(eval HEADERDEPS := $(CFILES:.c=.d))
	$(eval OBJS := $(CFILES:.c=.o))
	rm -f $(OBJS) blob.bin $(BIOS) $(HEADERDEPDS)

graph:
	cflow2dot -i $(CFILES) -f dot --source bios_main
	cat *.dot | gvpack -o svg.dot
	dot -Tsvg svg.dot > graph.svg
	rm *.dot
