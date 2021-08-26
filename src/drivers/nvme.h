#ifndef __DRIVERS_NVME_H__
#define __DRIVERS_NVME_H__

#include <stdint.h>

#define NVME_CLASS 0x01
#define NVME_SUBCLASS 0x08
#define NVME_INTERFACE 0x02

#define NVME_CMD_ADMIN_CREATE_ICQ 0x05
#define NVME_CMD_ADMIN_CREATE_ISQ 0x01
#define NVME_CMD_ADMIN_ID 0x06
#define NVME_CMD_ADMIN_ID_NAMESPACE 0
#define NVME_CMD_ADMIN_ID_CONTROLLER 1
#define NVME_CMD_ADMIN_ID_NAMESPACES 2

#define NVME_CFG_CAP_BOOT_PART (1 << 45)
#define NVME_CFG_CAP_MPSMIN_MASK 0x0f
#define NVME_CFG_CAP_MPSMIN_SHIFT 48
#define NVME_CFG_CAP_CSS_MASK 0xff
#define NVME_CFG_CAP_CSS_SHIFT 37
#define NVME_CFG_CAP_RESET ((uint64_t) 1 << 36)
#define NVME_CFG_CAP_CONTIGUOUS (1 << 16)
#define NVME_CFG_CAP_DSTRD_MASK 0x0f
#define NVME_CFG_CAP_DSTRD_SHIFT 32
#define NVME_CFG_CAP_MQES_MASK 0xffff
#define NVME_CFG_CC_IOCQES_SHIFT 20
#define NVME_CFG_CC_IOSQES_SHIFT 16
#define NVME_CFG_CC_AMS_SHIFT 11
#define NVME_CFG_CC_MPS_SHIFT 7
#define NVME_CFG_CC_CSS_SHIFT 4
#define NVME_CFG_CC_EN (1 << 0)
#define NVME_CFG_CS_RDY (1 << 0)
#define NVME_CFG_CS_CFS (1 << 1)
#define NVME_AQA_ACQS_SHIFT 16
#define NVME_AQA_ASQS_SHIFT 0
struct nvme_configuration {
    uint64_t capabilities;
    uint32_t version;
    uint32_t interrupt_mask_set;
    uint32_t interrupt_mask_clear;
    uint32_t controller_config; // cc
    uint32_t reserved;
    uint32_t controller_status;
    uint32_t nvme_reset;
    uint32_t admin_queue_attrs;
    uint64_t admin_submission_queue_addr;
    uint64_t admin_completion_queue_addr;
    uint32_t controller_mem_buffer_addr;
    uint32_t controller_mem_buffer_size;
    uint32_t boot_partition_info;
} __attribute__((__packed__));

struct nvme_submission_entry {
    uint8_t opcode;
    uint8_t flags;
    uint16_t command_id;
    uint32_t namespace_id;
    uint32_t reserved1;
    uint32_t reserved2;
    uint64_t metadata;
    uint64_t prp1;
    uint64_t prp2;
    uint32_t cmd_specific[6];
} __attribute__((__packed__));

#define NVME_C_ENT_STS_PHASE (1 << 0)
struct nvme_completion_entry {
    uint32_t command_specific;
    uint32_t reserved;
    uint16_t submission_entry_head;
    uint16_t submission_entry_id;
    uint16_t command_id;
    uint16_t status;
} __attribute__((__packed__));

void nvme_init();
int nvme_command(
    volatile struct nvme_configuration *cfg, struct nvme_submission_entry *command,
    volatile struct nvme_submission_entry *sq, volatile struct nvme_completion_entry *cq,
    int id, uint32_t *tail_ptr, uint32_t *head_ptr, int *phase);

#endif
