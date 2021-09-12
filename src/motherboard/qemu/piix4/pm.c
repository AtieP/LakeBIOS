#include <drivers/bus/pci.h>
#include <motherboard/qemu/piix4/pm.h>

static uint8_t pm_read_byte(uint8_t offset) {
    return pci_cfg_read_byte(QEMU_PIIX4_PM_BUS, QEMU_PIIX4_PM_SLOT, QEMU_PIIX4_PM_FUNCTION, offset);
}

static void pm_write_byte(uint8_t offset, uint8_t value) {
    pci_cfg_write_byte(QEMU_PIIX4_PM_BUS, QEMU_PIIX4_PM_SLOT, QEMU_PIIX4_PM_FUNCTION, offset, value);
}

static uint32_t pm_read_dword(uint8_t offset) {
    return pci_cfg_read_dword(QEMU_PIIX4_PM_BUS, QEMU_PIIX4_PM_SLOT, QEMU_PIIX4_PM_FUNCTION, offset);
}

static void pm_write_dword(uint8_t offset, uint32_t value) {
    pci_cfg_write_dword(QEMU_PIIX4_PM_BUS, QEMU_PIIX4_PM_SLOT, QEMU_PIIX4_PM_FUNCTION, offset, value);
}

/* Power Management Base Address */

void qemu_piix4_pm_pmba_set(uint16_t pmba) {
    pm_write_dword(QEMU_PIIX4_PM_PMBA, pmba & QEMU_PIIX4_PM_PMBA_MASK);
}

/* Device Activity */

void qemu_piix4_pm_devacta_set(uint32_t bits) {
    pm_write_dword(QEMU_PIIX4_PM_DEVACTA, pm_read_dword(QEMU_PIIX4_PM_DEVACTA) | bits);
}

void qemu_piix4_pm_devacta_clear(uint32_t bits) {
    pm_write_dword(QEMU_PIIX4_PM_DEVACTA, pm_read_dword(QEMU_PIIX4_PM_DEVACTA) & ~bits);
}

/* Power Management Miscelaneous Register */

void qemu_piix4_pm_pmregmisc_pmba_en() {
    pm_write_byte(QEMU_PIIX4_PM_PMREGMISC, QEMU_PIIX4_PM_PMREGMISC_PMIOSE);
}

void qemu_piix4_pm_pmregmisc_pmba_dis() {
    pm_write_byte(QEMU_PIIX4_PM_PMREGMISC, 0);
}

/* System Bus Base Adress */

void qemu_piix4_pm_smbba_set(uint16_t smbba) {
    pm_write_dword(QEMU_PIIX4_PM_SMBBA, smbba & QEMU_PIIX4_PM_SMBBA_MASK);
}

/* System Bus Host Configuration */

void qemu_piix4_pm_smbhstcfg_smbba_en() {
    pm_write_byte(QEMU_PIIX4_PM_SMBHSTCFG, pm_read_byte(QEMU_PIIX4_PM_SMBHSTCFG) | QEMU_PIIX4_PM_SMBHSTCFG_SMB_HST_EN);
}
void qemu_piix4_pm_smbhstcfg_smbba_dis() {
    pm_write_byte(QEMU_PIIX4_PM_SMBHSTCFG, pm_read_byte(QEMU_PIIX4_PM_SMBHSTCFG) & ~QEMU_PIIX4_PM_SMBHSTCFG_SMB_HST_EN);
}

void qemu_piix4_pm_smbhstcfg_smbba_irq(uint8_t irq) {
    pm_write_byte(QEMU_PIIX4_PM_SMBHSTCFG, (pm_read_byte(QEMU_PIIX4_PM_SMBHSTCFG) & ~QEMU_PIIX4_PM_SMBHSTCFG_IRQ_MASK) | irq);
}
