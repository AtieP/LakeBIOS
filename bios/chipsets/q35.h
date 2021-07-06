#ifndef __CHIPSETS_Q35_H__
#define __CHIPSETS_Q35_H__

#include <stdint.h>

#define Q35_PCI_MMIO_BAR_BASE 0xc0000000
#define Q35_PCI_IO_BAR_BASE 0xc000
#define Q35_PCIEXBAR_BASE 0xb0000000

#define Q35_DRAM_BUS 0
#define Q35_DRAM_SLOT 0
#define Q35_DRAM_FUNCTION 0
#define Q35_DRAM_VENDOR 0x8086
#define Q35_DRAM_DEVICE 0x29c0
#define Q35_DRAM_PCIEXBAR 0x60
#define Q35_DRAM_PAM_BASE 0x90
#define Q35_DRAM_SMRAM 0x9d
#define Q35_DRAM_TOLUD 0xb0

#define Q35_DRAM_SMRAM_OPEN (1 << 6)
#define Q35_DRAM_SMRAM_CLOSE (1 << 5)
#define Q35_DRAM_SMRAM_LOCK (1 << 4)
#define Q35_DRAM_SMRAM_EN (1 << 3)

#define Q35_LPC_BUS 0
#define Q35_LPC_SLOT 31
#define Q35_LPC_FUNCTION 0
#define Q35_LPC_PMBASE 0x40
#define Q35_LPC_ACPI_CNTL 0x44

#define Q35_LPC_ACPI_CNTL_EN (1 << 7)

void q35_init();
void q35_dram_shadow_bios();
void q35_dram_unshadow_bios();
void q35_dram_smram_open();
void q35_dram_smram_close();
void q35_dram_smram_lock();
void q35_dram_smram_enable();
void q35_dram_tolud_set(uintptr_t tolud);
void q35_dram_pciexbar_enable();
void q35_dram_pciexbar_disable();
void q35_dram_pciexbar_set(uint64_t pciexbar);

void q35_lpc_acpi_base(uint16_t base);
void q35_lpc_acpi_enable();
void q35_lpc_acpi_disable();

#endif
