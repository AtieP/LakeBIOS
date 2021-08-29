#ifndef __CHIPSETS_I440FX_HAL_H__
#define __CHIPSETS_I440FX_HAL_H__

#include <hal/power.h>

int i440fx_hal_power_reset(struct power_abstract *power_abstract);
int i440fx_hal_power_s3(struct power_abstract *power_abstract);
int i440fx_hal_power_s4(struct power_abstract *power_abstract);
int i440fx_hal_power_s5(struct power_abstract *power_abstract);

#endif
