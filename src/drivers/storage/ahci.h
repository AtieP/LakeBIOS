#ifndef __DRIVERS_STORAGE_AHCI_H__
#define __DRIVERS_STORAGE_AHCI_H__

#include <stdint.h>

#define AHCI_CLASS 0x01
#define AHCI_SUBCLASS 0x06
#define AHCI_INTERFACE 0x01

#define AHCI_FIS_H2D 0x27
struct ahci_fis_h2d {
    uint8_t fis_kind;
    uint8_t flags;
    uint8_t command;
    uint8_t features_low;
    uint8_t lba0;
    uint8_t lba1;
    uint8_t lba2;
    uint8_t device;
    uint8_t lba3;
    uint8_t lba4;
    uint8_t lba5;
    uint8_t features_hi;
    uint8_t count_low;
    uint8_t count_hi;
    uint8_t icc;
    uint8_t control;
    char reserved[4];
} __attribute__((__packed__));

struct ahci_fis_d2h {
    uint8_t fis_kind;
    uint8_t flags;
    uint8_t status;
    uint8_t error;
    uint8_t lba0;
    uint8_t lba1;
    uint8_t lba2;
    uint8_t device;
    uint8_t lba3;
    uint8_t lba4;
    uint8_t lba5;
    uint8_t reserved1;
    uint8_t count_low;
    uint8_t count_high;
    char reserved2[6];
} __attribute__((__packed__));

struct ahci_fis_pio_setup {
    uint8_t fis_kind;
    uint8_t flags;
    uint8_t status;
    uint8_t error;
    uint8_t lba0;
    uint8_t lba1;
    uint8_t lba2;
    uint8_t device;
    uint8_t lba3;
    uint8_t lba4;
    uint8_t lba5;
    uint8_t reserved1;
    uint8_t count_low;
    uint8_t count_high;
    uint8_t reserved2;
    uint8_t new_status;
    uint16_t transfer_count;
    char reserved3[2];
} __attribute__((__packed__));

struct ahci_fis_dma_setup {
    uint8_t fis_kind;
    uint8_t flags;
    char reserved1[2];
    uint64_t dma_buffer_id;
    uint32_t reserved2;
    uint32_t dma_buffer_offset;
    uint32_t transfer_count;
    char reserved3[4];
} __attribute__((__packed__));

struct ahci_fis_hba {
    struct ahci_fis_dma_setup dma_setup;
    char reserved1[0x20 - 0x1c];
    struct ahci_fis_pio_setup pio_setup;
    char reserved2[0x40 - 0x34];
    struct ahci_fis_d2h d2h;
    char reserved3[0x58 - 0x54];
    uint64_t sdbfis;
    char ufis[0xa0 - 0x60];
    char reserved[0x100 - 0xa0];
} __attribute__((__packed__));

struct ahci_prdt {
    uint32_t data_addr_low;
    uint32_t data_addr_hi;
    uint32_t reserved;
    uint32_t description;
} __attribute__((__packed__));

// CCC: Command Completion Coalescenting
// EM: Enclosure Management
#define AHCI_CAP_64 (1 << 31)
#define AHCI_CAP_SMPS (1 << 28)
#define AHCI_CAP_SSS (1 << 27)
#define AHCI_CAP_PORTS_MASK 0x1f
#define AHCI_CAP_SLOTS_MASK 0x1f
#define AHCI_GHC_CNT_AE (1 << 31)
#define AHCI_GHC_CNT_RESET (1 << 0)
#define AHCI_GHC_CAP_EXT_BIOS (1 << 0)
#define AHCI_GHC_BIOS_BOS (1 << 0)
struct ahci_ghc {
    uint32_t hba_capabilities;
    uint32_t global_hba_control;
    uint32_t interrupt_status;
    uint32_t ports;
    uint32_t version;
    uint32_t ccc_control;
    uint32_t ccc_ports;
    uint32_t em_location;
    uint32_t em_control;
    uint32_t hba_capabilities_ext;
    uint32_t bios_handoff_cnt_sts;
} __attribute__((__packed__));

#define AHCI_PORT_CMD_STS_CR (1 << 15)
#define AHCI_PORT_CMD_STS_FR (1 << 14)
#define AHCI_PORT_CMD_STS_FRE (1 << 4)
#define AHCI_PORT_CMD_STS_SUD (1 << 2)
#define AHCI_PORT_CMD_STS_ST (1 << 0)
#define AHCI_PORT_TFD_STS_DRQ (1 << 3)
#define AHCI_PORT_TFD_STS_BSY (1 << 7)
#define AHCI_PORT_SATA_STS_DET_MASK 0x0f
struct ahci_port {
    uint32_t commands_list_addr_low;
    uint32_t commands_list_addr_hi;
    uint32_t fis_addr_low;
    uint32_t fis_addr_hi;
    uint32_t interrupt_status;
    uint32_t interrupt_enable;
    uint32_t command_status;
    uint32_t reserved1;
    uint32_t task_file_data; // tfd
    uint32_t signature;
    uint32_t sata_status;
    uint32_t sata_control;
    uint32_t sata_error;
    uint32_t sata_active;
    uint32_t command_issue;
    uint32_t sata_notification;
    uint32_t fis_based_switching_cnt;
    uint32_t device_sleep;
    uint32_t reserved2[14];
} __attribute__((__packed__));

#define AHCI_CMD_HDR_FLAGS_W (1 << 6)
#define AHCI_CMD_HDR_FLAGS_ATAPI (1 << 5)
#define AHCI_CMD_HDR_FLAGS_PRDTL_SHIFT 16
struct ahci_command_hdr {
    uint32_t flags;
    uint32_t prdt_count;
    uint32_t command_table_low;
    uint32_t command_table_hi;
    uint32_t reserved[4];
} __attribute__((__packed__));

struct ahci_command_tbl {
    struct ahci_fis_h2d command_fis;
    char gap[64 - sizeof(struct ahci_fis_h2d)];
    char atapi[16];
    char reserved[48];
    struct ahci_prdt prdt[];
} __attribute__((__packed__));

struct ahci_abar {
    struct ahci_ghc ghc;
    char reserved[0x100 - 0x2c];
    struct ahci_port ports[32];
} __attribute__((__packed__));

void ahci_init();
int ahci_command(volatile struct ahci_abar *abar, int port, int write, int atapi, struct ahci_command_tbl *tbl, int prdt_len);

#endif
