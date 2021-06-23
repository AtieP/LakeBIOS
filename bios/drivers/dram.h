#ifndef __DRIVERS_DRAM_H__
#define __DRIVERS_DRAM_H__

#define DRAM_BUS 0
#define DRAM_SLOT 0
#define DRAM_FUNCTION 0

#define DRAM_PAM0 0x90
#define DRAM_PAM1 0x91
#define DRAM_PAM2 0x92
#define DRAM_PAM3 0x93
#define DRAM_PAM4 0x94
#define DRAM_PAM5 0x95
#define DRAM_PAM6 0x96

#define DRAM_PAM_LO_NO_DRAM (0 << 0)
#define DRAM_PAM_LO_RO (1 << 0)
#define DRAM_PAM_LO_WO (2 << 0)
#define DRAM_PAM_LO_DRAM (3 << 0)

#define DRAM_PAM_HI_NO_DRAM (0 << 4)
#define DRAM_PAM_HI_RO (1 << 4)
#define DRAM_PAM_HI_WO (2 << 4)
#define DRAM_PAM_HI_DRAM (3 << 4)

#define DRAM_SMRAM 0x9d

#define DRAM_SMRAM_OPEN (1 << 6)
#define DRAM_SMRAM_CLOSED (1 << 5)
#define DRAM_SMRAM_LOCKED (1 << 4)
#define DRAM_SMRAM_ENABLE (1 << 3)
#define DRAM_SMRAM_DEFAULT (0b010)

void dram_lock_bios();
void dram_unlock_bios();

void dram_open_smram();
void dram_close_smram();
void dram_lock_smram();
void dram_enable_smram();

#endif
