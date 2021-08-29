#ifndef __CHIPSETS_Q35_HAL_H__
#define __CHIPSETS_Q35_HAL_H__

#include <hal/power.h>

int q35_hal_power_reset(struct power_abstract *power_abstract);
int q35_hal_power_s3(struct power_abstract *power_abstract);
int q35_hal_power_s4(struct power_abstract *power_abstract);
int q35_hal_power_s5(struct power_abstract *power_abstract);

#endif
