#include <drivers/ata_common.h>

int ata_common_identify_is_lba48(const uint16_t *identify) {
    return identify[83] & (1 << 10);
}

uint64_t ata_common_identify_sectors(const uint16_t *identify, int lba48) {
    if (lba48) {
        return *((uint64_t *) &identify[100]);
    } else {
        return *((uint32_t *) &identify[60]);
    }
}
