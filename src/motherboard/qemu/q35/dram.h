#ifndef __MOTHERBOARD_QEMU_Q35_DRAM_H__
#define __MOTHERBOARD_QEMU_Q35_DRAM_H__

#include <stdint.h>

#define QEMU_Q35_PCIEXBAR 0xe0000000

#define QEMU_Q35_DRAM_BUS      0
#define QEMU_Q35_DRAM_SLOT     0
#define QEMU_Q35_DRAM_FUNCTION 0

#define QEMU_Q35_DRAM_VENDOR 0x8086
#define QEMU_Q35_DRAM_DEVICE 0x29c0

// Extended TSEG (QEMU specific)
#define QEMU_Q35_DRAM_EXT_TSEG_MBYTES       0x50
#define QEMU_Q35_DRAM_EXT_TSEG_MBYTES_QUERY 0xffff

#define QEMU_Q35_DRAM_PCIEXBAR       0x60
#define QEMU_Q35_DRAM_PCIEXBAR_EN    (1 << 0)
#define QEMU_Q35_DRAM_PCIEXBAR_64MB  0x00
#define QEMU_Q35_DRAM_PCIEXBAR_128MB 0x02
#define QEMU_Q35_DRAM_PCIEXBAR_256MB 0x04

#define QEMU_Q35_DRAM_PAM0    0x90
#define QEMU_Q35_DRAM_PAM0_EN 0x03

// Following QEMU specific registers are for
// requesting a 128KB big SMBASE.
#define QEMU_Q35_DRAM_SMBASE        0x9c
#define QEMU_Q35_DRAM_SMBASE_QUERY  0xff
#define QEMU_Q35_DRAM_SMBASE_EXISTS 0x01
#define QEMU_Q35_DRAM_SMBASE_LOCKED 0x02

#define QEMU_Q35_DRAM_SMRAM       0x9d
#define QEMU_Q35_DRAM_SMRAM_OPEN  (1 << 6)
#define QEMU_Q35_DRAM_SMRAM_CLOSE (1 << 5)
#define QEMU_Q35_DRAM_SMRAM_LOCK  (1 << 4)
#define QEMU_Q35_DRAM_SMRAM_EN    (1 << 3)

#define QEMU_Q35_DRAM_ESMRAMC         0x9e
#define QEMU_Q35_DRAM_ESMRAMC_EN      (1 << 7)
#define QEMU_Q35_DRAM_ESMRAMC_TSEG_1M 0x00
#define QEMU_Q35_DRAM_ESMRAMC_TSEG_2M 0x01
#define QEMU_Q35_DRAM_ESMRAMC_TSEG_8M 0x02

int qemu_q35_dram_tseg_get_size();
int qemu_q35_dram_tseg_set_size(int mbs);

void qemu_q35_dram_pciexbar(uint64_t pciexbar, int size_mb);

void qemu_q35_dram_pam_lock(int pam);
void qemu_q35_dram_pam_unlock(int pam);

void qemu_q35_dram_smram_en();
void qemu_q35_dram_smram_dis();
void qemu_q35_dram_smram_open();
void qemu_q35_dram_smram_close();
void qemu_q35_dram_smram_close_close();
void qemu_q35_dram_smram_lock();

void qemu_q35_dram_esmramc_en();
void qemu_q35_dram_esmramc_dis();

#endif
