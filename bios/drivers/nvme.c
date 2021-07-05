#include <stddef.h>
#include <drivers/nvme.h>
#include <drivers/pci.h>
#include <tools/alloc.h>
#include <tools/math.h>
#include <tools/print.h>
#include <tools/string.h>

#define ENTRIES 16

// Get doorbell stride
static int get_dstrd(volatile struct nvme_configuration *cfg) {
    return (cfg->capabilities >> NVME_CFG_CAP_DSTRD_SHIFT) & NVME_CFG_CAP_DSTRD_MASK;
}

// Get minimum page size
static int get_mpsmin(volatile struct nvme_configuration *cfg) {
    return (cfg->capabilities >> NVME_CFG_CAP_MPSMIN_SHIFT) & NVME_CFG_CAP_MPSMIN_MASK;
}

static int controller_init(uint8_t nvme_bus, uint8_t nvme_slot, uint8_t nvme_function) {
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
    void *asq = calloc(sizeof(struct nvme_submission_entry) * ENTRIES, alignment);
    if (!asq) {
        return -1;
    }
    void *acq = calloc(sizeof(struct nvme_completion_entry) * ENTRIES, alignment);
    if (!acq) {
        free(asq, sizeof(struct nvme_submission_entry) * ENTRIES);
        return -1;
    }
    cfg->admin_queue_attrs = ((ENTRIES - 1) << NVME_AQA_ACQS_SHIFT) | ((ENTRIES - 1) << NVME_AQA_ASQS_SHIFT);
    cfg->admin_submission_queue_addr = (uint64_t) (uintptr_t) asq;
    cfg->admin_completion_queue_addr = (uint64_t) (uintptr_t) acq;
    // Set configuration and enable the controller
    cfg->controller_config =
          (4 << NVME_CFG_CC_IOCQES_SHIFT)
        | (6 << NVME_CFG_CC_IOSQES_SHIFT)
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
    // TODO: IDENTIFY, I/O queues for sending reads and writes
    return 0;
}

void nvme_init() {
    print("atiebios: NVME: initializing controllers");
    for (size_t i = 0; i < SIZE_MAX; i++) {
        uint8_t nvme_bus;
        uint8_t nvme_slot;
        uint8_t nvme_function;
        if (pci_get_device(NVME_CLASS, NVME_SUBCLASS, NVME_INTERFACE, &nvme_bus, &nvme_slot, &nvme_function, i) == 0) {
            print("atiebios: NVME: controller found at bus %d slot %d function %d", nvme_bus, nvme_slot, nvme_function);
            if (controller_init(nvme_bus, nvme_slot, nvme_function) == -1) {
                print("atiebios: NVME: controller mentioned before has not been initialized successfully");
            } else {
                print("atiebios: NVME: controller mentioned before has been initialized successfully");
            }
        } else {
            break;
        }
    }
    print("atiebios: NVME: finished initializing controllers");
}

int nvme_command(
    volatile struct nvme_configuration *cfg, struct nvme_submission_entry *command,
    volatile struct nvme_submission_entry *sq, volatile struct nvme_completion_entry *cq,
    int id, uint32_t *tail_ptr, uint32_t *head_ptr
) {
    // TODO:
    // Poll for completion
    (void) cq;
    uint32_t tail = *tail_ptr;
    uint32_t head = *head_ptr;
    uint32_t stride = get_dstrd(cfg);
    volatile uint32_t *s_tail_doorbell = (volatile uint32_t *) ((uintptr_t) cfg + 0x1000 + (2 * id) * (4 << stride));
    volatile uint32_t *c_head_doorbell = (volatile uint32_t *) ((uintptr_t) cfg + 0x1000 + (2 * id + 1) * (4 << stride));
    sq += tail;
    memcpy((void *) sq, command, sizeof(struct nvme_submission_entry));
    // trigger command
    if (++tail == ENTRIES) {
        tail = 0;
    }
    *s_tail_doorbell = tail;
    if (++head == ENTRIES) {
        head = 0;
    }
    *c_head_doorbell = head;
    *tail_ptr = tail;
    *head_ptr = head;
    return 0;
}
