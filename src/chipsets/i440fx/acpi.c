#include <chipsets/i440fx/acpi.h>
#include <tools/wait.h>

int i440fx_acpi_reset() {
    uint8_t cf9 = inb(0xcf9) & ~0x0e;
    outb(0xcf9, cf9 | 0x02);
    udelay(50);
    outb(0xcf9, cf9 | 0x0e);
    udelay(50);
    return 0;
}

int i440fx_acpi_s3() {
    outw(I440FX_ACPI_PM1A_CNT, I440FX_ACPI_SLP_EN | (I440FX_ACPI_SLP_TYP_S3 << 10));
    return 0;
}

int i440fx_acpi_s4() {
    outw(I440FX_ACPI_PM1A_CNT, I440FX_ACPI_SLP_EN | (I440FX_ACPI_SLP_TYP_S4 << 10));
    return 0;
}

int i440fx_acpi_s5() {
    outw(I440FX_ACPI_PM1A_CNT, I440FX_ACPI_SLP_EN | (I440FX_ACPI_SLP_TYP_S5 << 10));
    return 0;
}
