#include <stddef.h>
#include <cpu/pio.h>
#include <drivers/floppy.h>
#include <drivers/rtc.h>
#include <tools/print.h>

static const char *kind_to_name(uint8_t kind) {
    switch (kind) {
        case CMOS_FLOPPY_360:
            return "360KB 5.25 floppy";
        case CMOS_FLOPPY_1200:
            return "1.2MB 5.25 floppy";
        case CMOS_FLOPPY_720:
            return "720KB floppy";
        case CMOS_FLOPPY_1440:
            return "1.44MB 3.5 floppy";
        case CMOS_FLOPPY_2880:
            return "2.88MB 3.5 floppy";
        default:
            return "Unknown floppy";
    }
}

static void lba_to_chs(uint32_t lba, uint8_t cylinders, uint8_t heads, uint8_t sectors, uint8_t *cylinder, uint8_t *head, uint8_t *sector) {
    (void) cylinders;
    *cylinder = lba / (heads * sectors);
    *head = (lba % (heads * sectors)) / sectors;
    *sector = (lba % (heads * sectors)) % sectors + 1;
}

static uint32_t chs_to_lba(uint8_t cylinders, uint8_t heads, uint8_t sectors, uint8_t cylinder, uint8_t head, uint8_t sector) {
    (void) cylinders;
    return ((cylinder * heads + head) * sectors) + sector - 1;
}

static int floppy_setup(uint16_t io_base, uint8_t kind) {
    (void) kind;
    // Only support version 0x90
    uint8_t version;
    if (floppy_command(io_base, FLOPPY_CMD_VERSION, NULL, 0, &version, 1, 0) != 0) {
        return -1;
    }
    if (version != 0x90) {
        return -1;
    }
    return 0;
}

void floppy_init() {
    print("atiebios: floppy: initializing floppies");
    uint8_t master_kind = rtc_get_floppy_master_kind();
    if (master_kind) {
        print("atiebios: floppy: found a %s in the master slot, initializing", kind_to_name(master_kind));
        if (floppy_setup(FLOPPY_BASE_MASTER, master_kind) != 0) {
            print("atiebios: floppy: master floppy has not been initialized successfully");
        } else {
            print("atiebios: floppy: master floppy has been initialized successfully");
        }
    } else {
        print("atiebios: floppy: no master floppy found");
    }
    uint8_t slave_kind = rtc_get_floppy_slave_kind();
    if (slave_kind) {
        print("atiebios: floppy: found a %s in the slave slot, initializing", kind_to_name(slave_kind));
        if (floppy_setup(FLOPPY_BASE_SLAVE, slave_kind) != 0) {
            print("atiebios: floppy: slave floppy has not been initialized successfully");
        } else {
            print("atiebios: floppy: slave floppy has been initialized successfully");
        }
    } else {
        print("atiebios: floppy: no slave floppy found");
    }
    print("atiebios: floppy: finished initializing floppies");
}

// Note: following code might not work in real hardware. QEMU's floppy interface is much better than real hardware
// WIP
int floppy_command(uint16_t io_base, uint8_t command, uint8_t *input, int input_len, uint8_t *output, int output_len, int skip_exec_state) {
    int failed = 0;
main_procedure:
    if (failed) {
        print("atiebios: floppy: sending command %x failed, trying again...", command);
    }
    uint8_t msr;
    msr = inb(io_base + FLOPPY_MSR);
    if (!(msr & FLOPPY_MSR_RQM) || msr & FLOPPY_MSR_DIO) {
        return -1; // Cannot send commands
    }
    outb(io_base + FLOPPY_DF, command);
    while (1) {
        msr = inb(io_base + FLOPPY_MSR);
        if (msr & FLOPPY_MSR_RQM) {
            break;
        }
        if (input_len) {
            if (!(msr & FLOPPY_MSR_DIO)) {
                outb(io_base + FLOPPY_DF, input[0]);
                break;
            }
        }
    }
    if (!skip_exec_state) {
        msr = inb(io_base + FLOPPY_MSR);
        if (!(msr & FLOPPY_MSR_NDMA)) {
            goto result_phase;
        }
        int i = 1;
        while (1) {
            while (1) {
                msr = inb(io_base + FLOPPY_MSR);
                if (msr & FLOPPY_MSR_RQM) {
                    break;
                }
            }
            for (; i < input_len; i++) {
                outb(io_base + FLOPPY_DF, input[i]);
                msr = inb(io_base + FLOPPY_MSR);
            }
            break;
        }
    }
result_phase:
    if (skip_exec_state) {
        while (1) {
            msr = inb(io_base + FLOPPY_MSR);
            if (msr & (FLOPPY_MSR_RQM | FLOPPY_MSR_DIO)) {
                break;
            }
        }
    }
    for (int i = 0; i < output_len; i++) {
        output[i] = inb(io_base + FLOPPY_DF);
        msr = inb(io_base + FLOPPY_MSR);
        if (msr & FLOPPY_MSR_RQM) {
            break;
        }
    }
    msr = inb(io_base + FLOPPY_MSR);
    if (!(msr & FLOPPY_MSR_RQM) || msr & FLOPPY_MSR_DIO) {
        failed = 1;
        goto main_procedure;
    }
    return 0;
}
