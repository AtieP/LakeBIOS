#include <chipsets/q35/acpi.h>
#include <tools/wait.h>

int q35_acpi_reset() {
    uint8_t cf9 = inb(0xcf9) & ~0x0e;
    outb(0xcf9, cf9 | 0x02);
    udelay(50);
    outb(0xcf9, cf9 | 0x0e);
    udelay(50);
    return 0;
}

int q35_acpi_s3() {
    outw(Q35_ACPI_PM1A_CNT, Q35_ACPI_SLP_EN | (Q35_ACPI_SLP_TYP_S3 << 10));
    return 0;
}

int q35_acpi_s4() {
    outw(Q35_ACPI_PM1A_CNT, Q35_ACPI_SLP_EN | (Q35_ACPI_SLP_TYP_S4 << 10));
    return 0;
}

int q35_acpi_s5() {
    outw(Q35_ACPI_PM1A_CNT, Q35_ACPI_SLP_EN | (Q35_ACPI_SLP_TYP_S5 << 10));
    return 0;
}

