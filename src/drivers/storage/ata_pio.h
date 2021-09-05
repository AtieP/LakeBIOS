#ifndef __DRIVERS_STORAGE_ATA_PIO_H__
#define __DRIVERS_STORAGE_ATA_PIO_H__

#define ATA_PIO_PRIMARY_BASE 0x1f0
#define ATA_PIO_SECONDARY_BASE 0x170
#define ATA_PIO_TERTIARY_BASE 0x1e8
#define ATA_PIO_QUATERNARY_BASE 0x168

#define ATA_PIO_DATA(base) (base + 0)
#define ATA_PIO_ERROR(base) (base + 1)
#define ATA_PIO_FEATS(base) (base + 1)
#define ATA_PIO_SECTORS(base) (base + 2)
#define ATA_PIO_LBA_LOW(base) (base + 3)
#define ATA_PIO_SECTOR(base) (base + 3)
#define ATA_PIO_LBA_MID(base) (base + 4)
#define ATA_PIO_CYL_LOW(base) (base + 4)
#define ATA_PIO_LBA_HI(base) (base + 5)
#define ATA_PIO_CYL_HI(base) (base + 5)
#define ATA_PIO_LBA_DRIVE(base) (base + 6)
#define ATA_PIO_HEAD(base) (base + 6)
#define ATA_PIO_STATUS(base) (base + 7)
#define ATA_PIO_CMD(base) (base + 7)

#define ATA_PIO_ERROR_AMNF (1 << 0)
#define ATA_PIO_ERROR_TKZNF (1 << 1)
#define ATA_PIO_ERROR_ABRT (1 << 2)
#define ATA_PIO_ERROR_MCR (1 << 3)
#define ATA_PIO_ERROR_IDNF (1 << 4)
#define ATA_PIO_ERROR_MC (1 << 5)
#define ATA_PIO_ERROR_UNC (1 << 6)
#define ATA_PIO_ERROR_BBK (1 << 7)

#define ATA_PIO_STATUS_ERR (1 << 0)
#define ATA_PIO_STATUS_IDX (1 << 1)
#define ATA_PIO_STATUS_CORR (1 << 2)
#define ATA_PIO_STATUS_DRQ (1 << 3)
#define ATA_PIO_STATUS_SRV (1 << 4)
#define ATA_PIO_STATUS_DF (1 << 5)
#define ATA_PIO_STATUS_RDY (1 << 6)
#define ATA_PIO_STATUS_BSY (1 << 7)

#endif
