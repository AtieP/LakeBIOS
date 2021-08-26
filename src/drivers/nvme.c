#include <stddef.h>
#include <drivers/nvme.h>
#include <drivers/pci.h>
#include <hal/disk.h>
#include <tools/alloc.h>
#include <tools/math.h>
#include <tools/print.h>
#include <tools/string.h>

// TODO:
// 1. Make the IO queues shareable between namespaces of a controller. Right now they will just collide

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

// Reset controller
static void controller_reset(volatile struct nvme_configuration *cfg) {
    print("NVME: Resetting controller...");
    cfg->controller_config &= ~NVME_CFG_CC_EN;
    while (cfg->controller_status & NVME_CFG_CS_RDY);
}

static int hal_submit(struct disk_abstract *disk, int flp);

static int controller_init(uint8_t nvme_bus, uint8_t nvme_slot, uint8_t nvme_function) {
    pci_control_set(nvme_bus, nvme_slot, nvme_function, PCI_CFG_COMMAND_BUS_MASTER | PCI_CFG_COMMAND_IO_ENABLE | PCI_CFG_COMMAND_MEM_ENABLE);
    volatile struct nvme_configuration *cfg = (volatile struct nvme_configuration *) (uintptr_t) pci_get_bar(nvme_bus, nvme_slot, nvme_function, 0);
    if (!cfg) {
        return -1;
    }
    // Get alignments
    int mpsmin = get_mpsmin(cfg);
    int alignment = pow(2, 12 + mpsmin);
    // Before continuing, allocate all required buffers
    void *asq = calloc(sizeof(struct nvme_submission_entry) * ADMIN_ENTRIES, alignment);
    void *acq = calloc(sizeof(struct nvme_completion_entry) * ADMIN_ENTRIES, alignment);
    void *isq = calloc(sizeof(struct nvme_submission_entry) * IO_ENTRIES, alignment);
    void *icq = calloc(sizeof(struct nvme_completion_entry) * IO_ENTRIES, alignment);
    void *identify = calloc(4096, alignment);
    if (!asq) {
        print("NVME: Could not allocate the Admin Submission Queue");
        goto free;
    }
    if (!acq) {
        print("NVME: Could not allocate the Admin Completion Queue");
        goto free;
    }
    if (!isq) {
        print("NVME: Could not allocate the IO Submission Queue");
        goto free;
    }
    if (!icq) {
        print("NVME: Could not allocate the IO Completion Queue");
        goto free;
    }
    if (!identify) {
        print("NVME: Could not allocate the IDENTIFY buffer");
        goto free;
    }
    // Initialize controller
    int not_initialized_namespaces = 0;
    controller_reset(cfg);
    // At minimum the NVM command set is required
    uint8_t command_set = (cfg->capabilities >> NVME_CFG_CAP_CSS_SHIFT) & NVME_CFG_CAP_CSS_MASK;
    if (!(command_set & 1)) {
        goto free;
    }
    // Set the admin queues
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
            goto free;
        }
    }
    // Setup namespaces
    uint32_t admin_head = 0;
    uint32_t admin_tail = 0;
    int admin_phase = 1;
    uint32_t io_head = 0;
    uint32_t io_tail = 0;
    int io_phase = 1;
    uint32_t namespaces;
    uint32_t *namespace_list;
    struct nvme_submission_entry cmd;
    // 1. Identify the controller to know its namespaces
    memset(&cmd, 0, sizeof(struct nvme_submission_entry));
    cmd.opcode = NVME_CMD_ADMIN_ID;
    cmd.prp1 = (uint64_t) (uintptr_t) identify;
    cmd.cmd_specific[0] = NVME_CMD_ADMIN_ID_CONTROLLER;
    if (nvme_command(cfg, &cmd, asq, acq, 0, &admin_tail, &admin_head, &admin_phase) != 0) {
        print("NVME: Could not send IDENTIFY command to the controller");
        goto free;
    }
    namespaces = *((uint32_t *) ((uintptr_t) identify + 516));
    if (!namespaces) {
        print("NVME: Controller has no namespaces");
        goto free;
    }
    // 2. Query the namespace list
    namespace_list = calloc(namespaces * sizeof(uint32_t), alignment);
    if (!namespace_list) {
        print("NVME: Could not allocate the namespace list");
        goto free;
    }
    memset(&cmd, 0, sizeof(struct nvme_submission_entry));
    cmd.opcode = NVME_CMD_ADMIN_ID;
    cmd.prp1 = (uint64_t) (uintptr_t) namespace_list;
    cmd.cmd_specific[0] = NVME_CMD_ADMIN_ID_NAMESPACES;
    if (nvme_command(cfg, &cmd, asq, acq, 0, &admin_tail, &admin_head, &admin_phase) != 0) {
        print("NVME: Could not query the namespace list");
        goto free;
    }
    // 3.   Set up the IO queues
    // 3.1. Set up the IO Completion Queue
    memset(&cmd, 0, sizeof(struct nvme_submission_entry));
    cmd.opcode = NVME_CMD_ADMIN_CREATE_ICQ;
    cmd.prp1 = (uint64_t) (uintptr_t) icq;
    cmd.cmd_specific[0] = ((IO_ENTRIES - 1) << 16) | 1;
    cmd.cmd_specific[1] = 1 << 0; // Physically contiguous
    if (nvme_command(cfg, &cmd, asq, acq, 0, &admin_tail, &admin_head, &admin_phase) != 0) {
        print("NVME: Could not set the IO Completion Queue");
        goto free;
    }
    // 3.2. Set up the IO Submission Queue
    memset(&cmd, 0, sizeof(struct nvme_submission_entry));
    cmd.opcode = NVME_CMD_ADMIN_CREATE_ISQ;
    cmd.prp1 = (uint64_t) (uintptr_t) isq;
    cmd.cmd_specific[0] = ((IO_ENTRIES - 1) << 16) | 1;
    cmd.cmd_specific[1] = (1 << 16) | (1 << 0); // Physically contiguous
    if (nvme_command(cfg, &cmd, asq, acq, 0, &admin_tail, &admin_head, &admin_phase) != 0) {
        print("NVME: Could not set the IO Submission Queue");
        goto free;
    }
    // 4. Set up all the individual namespaces
    for (uint32_t i = 0; i < namespaces; i++) {
        // 4.1. Identify the namespace
        memset(&cmd, 0, sizeof(struct nvme_submission_entry));
        cmd.opcode = NVME_CMD_ADMIN_ID;
        cmd.prp1 = (uint64_t) (uintptr_t) identify;
        cmd.cmd_specific[0] = NVME_CMD_ADMIN_ID_NAMESPACE;
        cmd.namespace_id = namespace_list[i];
        if (nvme_command(cfg, &cmd, asq, acq, 0, &io_tail, &io_head, &io_phase) != 0) {
            print("NVME: Could not send IDENTIFY command to namespace #%d", namespace_list[i]);
            not_initialized_namespaces++;
            continue;
        }
        // 4.2. Submit it to the HAL
        struct disk_abstract disk = {0};
        disk.interface = HAL_DISK_NVME;
        disk.common.lba_max = (uint64_t) *((uint32_t *) identify + 0);
        disk.common.heads_per_cylinder = 16;
        disk.common.sectors_per_head = 255;
        disk.specific.nvme.cfg = cfg;
        disk.specific.nvme.cq = icq;
        disk.specific.nvme.sq = isq;
        disk.specific.nvme.namespace_id = namespace_list[i];
        disk.specific.nvme.queue_id = 1;
        disk.specific.nvme.head = io_head;
        disk.specific.nvme.tail = io_tail;
        disk.specific.nvme.phase = io_phase;
        if (hal_submit(&disk, 0) == HAL_DISK_ENOMORE) {
            break;
        }
    }
    if ((uint32_t) not_initialized_namespaces == namespaces) {
        goto free;
    } else {
        goto success;
    }
free:
    controller_reset(cfg);
    if (asq) {
        free(asq, sizeof(struct nvme_submission_entry) * ADMIN_ENTRIES);
    }
    if (acq) {
        free(acq, sizeof(struct nvme_completion_entry) * ADMIN_ENTRIES);
    }
    if (isq) {
        free(isq, sizeof(struct nvme_submission_entry) * IO_ENTRIES);
    }
    if (icq) {
        free(icq, sizeof(struct nvme_completion_entry) * IO_ENTRIES);
    }
    if (identify) {
        free(identify, 4096);
    }
    return -1;
success:
    free(identify, 4096);
    return not_initialized_namespaces;
}

void nvme_init() {
    print("NVME: Initializing controllers");
    for (size_t i = 0; i < SIZE_MAX; i++) {
        uint8_t nvme_bus;
        uint8_t nvme_slot;
        uint8_t nvme_function;
        if (pci_get_device(NVME_CLASS, NVME_SUBCLASS, NVME_INTERFACE, &nvme_bus, &nvme_slot, &nvme_function, i) == 0) {
            print("NVME: Controller found at PCI Bus %d Slot %d Function %d", nvme_bus, nvme_slot, nvme_function);
            int ret = controller_init(nvme_bus, nvme_slot, nvme_function);
            if (ret == -1) {
                print("NVME: Could not initialize previous controller at all");
            } else if (ret == 0) {
                print("NVME: Previous controller and all of its namespaces initialized successfully");
            } else {
                print("NVME: Previous controller initialized successfully, but %d of its namespaces weren't set up", ret);
            }
        } else {
            break;
        }
    }
    print("NVME: Finished initializing controllers");
}

#include <tools/wait.h>

int nvme_command(
    volatile struct nvme_configuration *cfg, struct nvme_submission_entry *command,
    volatile struct nvme_submission_entry *sq, volatile struct nvme_completion_entry *cq,
    int id, uint32_t *tail_ptr, uint32_t *head_ptr, int *phase
) {
    // Some variables and get locations of the doorbells
    uint32_t stride = get_dstrd(cfg);
    volatile uint32_t *s_tail_doorbell = (volatile uint32_t *) ((uintptr_t) cfg + 0x1000 + ((2 * id) * (4 << stride)));
    volatile uint32_t *c_head_doorbell = (volatile uint32_t *) ((uintptr_t) cfg + 0x1000 + ((2 * id + 1) * (4 << stride)));
    // Send the command
    memcpy((void *) &sq[*tail_ptr], command, sizeof(struct nvme_submission_entry));
    *tail_ptr = *tail_ptr + 1;
    if (*tail_ptr == IO_ENTRIES) {
        *tail_ptr = 0;
    }
    *s_tail_doorbell = *tail_ptr;
    // Wait for it to finish
    while (!((cq[*head_ptr].status & NVME_C_ENT_STS_PHASE) == *phase));
    if (cq[*head_ptr].status >> 1) {
        return -1;
    }
    // Update head
    *head_ptr = *head_ptr + 1;
    if (*head_ptr == IO_ENTRIES) {
        *head_ptr = 0;
        *phase = !*phase;
    }
    *c_head_doorbell = *head_ptr;
    return 0;
}

static int hal_rw(struct disk_abstract *this, void *buf, uint64_t lba, int len, int write) {
    struct nvme_submission_entry cmd = {0};
    cmd.opcode = write ? 0x01 : 0x02;
    cmd.namespace_id = this->specific.nvme.namespace_id;
    cmd.prp1 = (uint64_t) (uintptr_t) buf;
    cmd.cmd_specific[0] = (uint32_t) lba;
    cmd.cmd_specific[1] = (uint32_t) (lba >> 32);
    cmd.cmd_specific[2] = (uint16_t) (len / 512) - 1;
    return nvme_command(
        this->specific.nvme.cfg,
        &cmd,
        this->specific.nvme.sq,
        this->specific.nvme.cq,
        this->specific.nvme.queue_id,
        &this->specific.nvme.tail,
        &this->specific.nvme.head,
        &this->specific.nvme.phase
    ) != 0 ? HAL_DISK_EUNK : HAL_DISK_ESUCCESS;
}

static int hal_submit(struct disk_abstract *disk, int flp) {
    disk->ops.rw = hal_rw;
    return hal_disk_submit(disk, flp);
}
