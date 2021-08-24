#ifndef __CHIPSETS_Q35_DRAM_H__
#define __CHIPSETS_Q35_DRAM_H__

#define Q35_DRAM_BUS 0
#define Q35_DRAM_SLOT 0
#define Q35_DRAM_FUNCTION 0

#define Q35_PCI_MMIO_BASE 0xc0000000
#define Q35_PCI_IO_BASE 0x1000

#define Q35_DRAM_PAM0 0x90
#define Q35_DRAM_PAM_UNLOCK 0x03

#define Q35_DRAM_SMRAM 0x9d
#define Q35_DRAM_SMRAM_OPEN (1 << 6)
#define Q35_DRAM_SMRAM_CLOSE (1 << 5)
#define Q35_DRAM_SMRAM_LOCK (1 << 4)
#define Q35_DRAM_SMRAM_EN (1 << 3)

void q35_dram_pam_lock(int pam);
void q35_dram_pam_unlock(int pam, int copy_to_ram);

void q35_dram_smram_open();
void q35_dram_smram_close();
void q35_dram_smram_lock();

#endif
