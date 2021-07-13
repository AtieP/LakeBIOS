#include <stddef.h>
#include <drivers/nvme.h>
#include <drivers/pci.h>
#include <hal/disk.h>
#include <tools/alloc.h>
#include <tools/math.h>
#include <tools/print.h>
#include <tools/string.h>

#define ADMIN_ENTRIES 16
#define IO_ENTRIES 32

// Get doorbell stride
static int get_dstrd(volatile struct nvme_configuration *cfg) {
    return (cfg->capabilities >> NVME_CFG_CAP_DSTRD_SHIFT) & NVME_CFG_CAP_DSTRD_MASK;
}

// Get minimum page size
static int get_mpsmin(volatile struct nvme_configuration *cfg) {
    return (cfg->capabilities >> NVME_CFG_CAP_MPSMIN_SHIFT) & NVME_CFG_CAP_MPSMIN_MASK;
}

static int controller_init(uint8_t nvme_bus, uint8_t nvme_slot, uint8_t nvme_function) {
    int not_initialized_namespaces = 0;
    pci_enable_bus_mastering(nvme_bus, nvme_slot, nvme_function);
    volatile struct nvme_configuration *cfg = (volatile struct nvme_configuration *) (uintptr_t) pci_get_bar(nvme_bus, nvme_slot, nvme_function, 0);
    if (!cfg) {
        return -1;
    }
    if (cfg->controller_config & NVME_CFG_CC_EN) {
        cfg->controller_config &= ~NVME_CFG_CC_EN;
    }
    while (cfg->controller_status & NVME_CFG_CS_RDY);
    // At minimum the NVM command set is required
    uint8_t command_set = (cfg->capabilities >> NVME_CFG_CAP_CSS_SHIFT) & NVME_CFG_CAP_CSS_MASK;
    if (!(command_set & 1)) {
        return -1;
    }
    // Allocate the admin queues and indicate their size
    int mpsmin = get_mpsmin(cfg);
    int alignment = pow(2, 12 + mpsmin);
    void *asq = calloc(sizeof(struct nvme_submission_entry) * ADMIN_ENTRIES, alignment);
    if (!asq) {
        return -1;
    }
    void *acq = calloc(sizeof(struct nvme_completion_entry) * ADMIN_ENTRIES, alignment);
    if (!acq) {
        free(asq, sizeof(struct nvme_submission_entry) * ADMIN_ENTRIES);
        return -1;
    }
    cfg->admin_queue_attrs = ((ADMIN_ENTRIES - 1) << NVME_AQA_ACQS_SHIFT) | ((ADMIN_ENTRIES - 1) << NVME_AQA_ASQS_SHIFT);
    cfg->admin_submission_queue_addr = (uint64_t) (uintptr_t) asq;
    cfg->admin_completion_queue_addr = (uint64_t) (uintptr_t) acq;
    // Set configuration and enable the controller
    cfg->controller_config =
          (4 << NVME_CFG_CC_IOCQES_SHIFT) // 2^4 = 16: Completion entry size
        | (6 << NVME_CFG_CC_IOSQES_SHIFT) // 2^6 = 64: Submission entry size
        | (mpsmin << NVME_CFG_CC_MPS_SHIFT)
        | (0 << NVME_CFG_CC_CSS_SHIFT) // NVM command set
        | (0 << NVME_CFG_CC_AMS_SHIFT) // Round robin
        | NVME_CFG_CC_EN
    ;
    // Wait until it has been enabled
    for (;;) {
        if (cfg->controller_status & NVME_CFG_CS_RDY) {
            break;
        } else if (cfg->controller_status & NVME_CFG_CS_CFS) {
            return -1;
        }
    }
    // Allocate queues and buffers
    void *identify_buffer = calloc(4096, alignment);
    if (!identify_buffer) {
        return -1;
    }
    void *io_completion_queue = calloc(sizeof(struct nvme_completion_entry) * IO_ENTRIES, alignment);
    if (!io_completion_queue) {
        free(identify_buffer, 4096);
        return -1;
    }
    void *io_submission_queue = calloc(sizeof(struct nvme_submission_entry) * IO_ENTRIES, alignment);
    if (!io_submission_queue) {
        free(identify_buffer, 4096);
        free(io_completion_queue, sizeof(struct nvme_completion_entry) * IO_ENTRIES);
        return -1;
    }
    uint32_t head = 0;
    uint32_t tail = 0;
    // Identify controller
    struct nvme_submission_entry cmd;
    memset(&cmd, 0, sizeof(struct nvme_submission_entry));
    cmd.opcode = NVME_CMD_ADMIN_ID;
    cmd.prp1 = (uint32_t) identify_buffer;
    cmd.cmd_specific[0] = NVME_CMD_ADMIN_ID_CONTROLLER;
    if (nvme_command(cfg, &cmd, asq, acq, 0, &tail, &head) == -1) {
        goto free_error;
    }
    uint32_t namespaces = *((uint32_t *) (identify_buffer + 516));
    // Get namespace list
    uint32_t *namespace_list = calloc(sizeof(uint32_t) * namespaces, alignment);
    if (!namespace_list) {
        goto free_error;
    }
    memset(&cmd, 0, sizeof(struct nvme_submission_entry));
    cmd.opcode = NVME_CMD_ADMIN_ID;
    cmd.prp1 = (uint32_t) namespace_list;
    cmd.cmd_specific[0] = NVME_CMD_ADMIN_ID_NAMESPACES;
    if (nvme_command(cfg, &cmd, asq, acq, 0, &tail, &head) == -1) {
        goto free_error;
    }
    // Create queues
    memset(&cmd, 0, sizeof(struct nvme_submission_entry));
    cmd.opcode = NVME_CMD_ADMIN_CREATE_ICQ;
    cmd.prp1 = (uint64_t) (uintptr_t) io_completion_queue;
    cmd.cmd_specific[0] = ((sizeof(struct nvme_completion_entry) * IO_ENTRIES - 1) << 16) | 1;
    cmd.cmd_specific[1] = 1 << 0; // Physically contiguous
    if (nvme_command(cfg, &cmd, asq, acq, 0, &tail, &head) == -1) {
        goto free_error;
    }
    memset(&cmd, 0, sizeof(struct nvme_submission_entry));
    cmd.opcode = NVME_CMD_ADMIN_CREATE_ISQ;
    cmd.prp1 = (uint64_t) (uintptr_t) io_submission_queue;
    cmd.cmd_specific[0] = ((sizeof(struct nvme_submission_entry) * IO_ENTRIES - 1) << 16) | 1;
    cmd.cmd_specific[1] = (1 << 16) | (1 << 0); // Physically contiguous
    if (nvme_command(cfg, &cmd, asq, acq, 0, &tail, &head) == -1) {
        // whatever...
        free(identify_buffer, 4096);
        free(io_submission_queue, sizeof(struct nvme_submission_entry) * IO_ENTRIES);
        return -1;
    }
    // Finally identify the namespaces and submit them to the HAL
    for (uint32_t i = 0; i < namespaces; i++) {
        if (!namespace_list[i]) {
            continue;
        }
        memset(&cmd, 0, sizeof(struct nvme_submission_entry));
        cmd.opcode = NVME_CMD_ADMIN_ID;
        cmd.prp1 = (uint64_t) (uintptr_t) identify_buffer;
        cmd.cmd_specific[0] = NVME_CMD_ADMIN_ID_NAMESPACE;
        cmd.namespace_id = namespace_list[i];
        if (nvme_command(cfg, &cmd, asq, acq, 0, &tail, &head) == -1) {
            not_initialized_namespaces++;
            continue;
        }
        struct disk_abstract disk;
        disk.common.lba_max = *((uint32_t *) (identify_buffer + 0));
        disk.interface = HAL_DISK_NVME;
        disk.specific.nvme.cfg = cfg;
        disk.specific.nvme.cq = io_completion_queue;
        disk.specific.nvme.sq = io_submission_queue;
        disk.specific.nvme.namespace_id = namespace_list[i];
        disk.specific.nvme.queue_id = 1;
        disk.specific.nvme.head = 0;
        disk.specific.nvme.tail = 0;
        hal_disk_submit(&disk);
    }
    free(namespace_list, namespaces * sizeof(uint32_t));
    goto free_success;
free_error:
    free(io_completion_queue, sizeof(struct nvme_completion_entry) * IO_ENTRIES);
    free(io_submission_queue, sizeof(struct nvme_submission_entry) * IO_ENTRIES);
    free(identify_buffer, 4096);
    return -1;

free_success:
    free(identify_buffer, 4096);
    return not_initialized_namespaces;
}

void nvme_init() {
    print("lakebios: NVME: initializing controllers");
    for (size_t i = 0; i < SIZE_MAX; i++) {
        uint8_t nvme_bus;
        uint8_t nvme_slot;
        uint8_t nvme_function;
        if (pci_get_device(NVME_CLASS, NVME_SUBCLASS, NVME_INTERFACE, &nvme_bus, &nvme_slot, &nvme_function, i) == 0) {
            print("lakebios: NVME: controller found at bus %d slot %d function %d", nvme_bus, nvme_slot, nvme_function);
            int ret = controller_init(nvme_bus, nvme_slot, nvme_function);
            if (ret == -1) {
                print("lakebios: NVME: error during initializing the controller mentioned before. Caused by an allocation failure, or a controller error.");
            } else if (ret == 0) {
                print("lakebios: NVME: success during initializing the controller mentioned before");
            } else {
                print("lakebios: NVME: success during initializing the controller mentioned before, but %d namespaces were not set up. Caused by an allocation failure, or controller error.");
            }
        } else {
            break;
        }
    }
    print("lakebios: NVME: finished initializing controllers");
}

int nvme_command(
    volatile struct nvme_configuration *cfg, struct nvme_submission_entry *command,
    volatile struct nvme_submission_entry *sq, volatile struct nvme_completion_entry *cq,
    int id, uint32_t *tail_ptr, uint32_t *head_ptr
) {
    uint32_t tail = *tail_ptr;
    uint32_t head = *head_ptr;
    uint32_t stride = get_dstrd(cfg);
    volatile uint32_t *s_tail_doorbell = (volatile uint32_t *) ((uintptr_t) cfg + 0x1000 + (2 * id) * (4 << stride));
    volatile uint32_t *c_head_doorbell = (volatile uint32_t *) ((uintptr_t) cfg + 0x1000 + (2 * id + 1) * (4 << stride));
    sq += tail;
    memcpy((void *) sq, command, sizeof(struct nvme_submission_entry));
    // Trigger command
    if (++tail == IO_ENTRIES) {
        tail = 0;
    }
    *s_tail_doorbell = tail;
    // Wait for completion, basically it is impossible
    // to have the status field clear, and if a command
    // has been completed successfully, the 0th bit is set,
    // otherwise some other bit
    while (!(cq[head].status & 1));
    // Check if an error happened
    if (cq[head].status >> 1) {
        return -1;
    }
    if (++head == IO_ENTRIES) {
        head = 0;
    }
    *c_head_doorbell = head;
    *tail_ptr = tail;
    *head_ptr = head;
    return 0;
}
