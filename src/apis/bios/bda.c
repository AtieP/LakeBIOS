#include <apis/bios/bda.h>

static volatile void *bda = (void *) APIS_BIOS_BDA;

void api_bios_bda_com_set(int com, uint16_t addr) {
    *((volatile uint16_t *) bda + (com - 1)) = addr;
}

void api_bios_bda_lpt_set(int lpt, uint16_t addr) {
    *((volatile uint16_t *) bda + (lpt - 1)) = addr;
}

uint16_t api_bios_bda_com_get(int com) {
    return *((volatile uint16_t *) bda + (com - 1));
}

uint16_t api_bios_bda_lpt_get(int lpt) {
    return *((volatile uint16_t *) bda + (lpt - 1));
}

void api_bios_bda_floppy_status_set(uint8_t status) {
    *((volatile uint8_t *) (bda + 0x41)) = status;       
}

void api_bios_bda_disk_status_set(uint8_t status) {
    *((volatile uint8_t *) (bda + 0x74)) = status;
}

uint8_t api_bios_bda_floppy_status_get() {
    return *((volatile uint8_t *) (bda + 0x41));
}

uint8_t api_bios_bda_disk_status_get() {
    return *((volatile uint8_t *) (bda + 0x74));
}
