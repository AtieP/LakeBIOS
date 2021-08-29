#include <chipsets/i440fx/acpi.h>
#include <chipsets/i440fx/hal.h>
#include <hal/power.h>

int i440fx_hal_power_reset(struct power_abstract *power_abstract) {
    (void) power_abstract;
    return i440fx_acpi_reset() != 0 ? HAL_POWER_EUNK : HAL_POWER_ESUCCESS;
}

int i440fx_hal_power_s3(struct power_abstract *power_abstract) {
    (void) power_abstract;
    return i440fx_acpi_s3() != 0 ? HAL_POWER_EUNK : HAL_POWER_ESUCCESS;
}

int i440fx_hal_power_s4(struct power_abstract *power_abstract) {
    (void) power_abstract;
    return i440fx_acpi_s4() != 0 ? HAL_POWER_EUNK : HAL_POWER_ESUCCESS;
}

int i440fx_hal_power_s5(struct power_abstract *power_abstract) {
    (void) power_abstract;
    return i440fx_acpi_s5() != 0 ? HAL_POWER_EUNK : HAL_POWER_ESUCCESS;
}
