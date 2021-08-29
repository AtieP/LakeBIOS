#include <chipsets/q35/acpi.h>
#include <chipsets/q35/hal.h>
#include <hal/power.h>

int q35_hal_power_reset(struct power_abstract *power_abstract) {
    (void) power_abstract;
    return q35_acpi_reset() != 0 ? HAL_POWER_EUNK : HAL_POWER_ESUCCESS;
}

int q35_hal_power_s3(struct power_abstract *power_abstract) {
    (void) power_abstract;
    return q35_acpi_s3() != 0 ? HAL_POWER_EUNK : HAL_POWER_ESUCCESS;
}

int q35_hal_power_s4(struct power_abstract *power_abstract) {
    (void) power_abstract;
    return q35_acpi_s4() != 0 ? HAL_POWER_EUNK : HAL_POWER_ESUCCESS;
}

int q35_hal_power_s5(struct power_abstract *power_abstract) {
    (void) power_abstract;
    return q35_acpi_s5() != 0 ? HAL_POWER_EUNK : HAL_POWER_ESUCCESS;
}
