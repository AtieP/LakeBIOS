#include <hal/power.h>
#include <tools/print.h>
#include <tools/string.h>

static struct power_abstract power_module = {0};

static const char *power_module_type_to_name(int interface) {
    switch (interface) {
        case HAL_POWER_I440FX:
            return "i440fx Power Management module";
        case HAL_POWER_Q35:
            return "Q35 Power Management module";
        default:
            return "Unknown Power Management module";
    }
}

int hal_power_submit(struct power_abstract *power_abstract) {
    print("HAL: Submitting a: %s", power_module_type_to_name(power_abstract->interface));
    memcpy(&power_module, power_abstract, sizeof(struct power_abstract));
    power_module.present = 1;
    return HAL_POWER_ESUCCESS;
}

int hal_power_reset() {
    if (!power_module.present) {
        return HAL_POWER_ENOFOUND;
    }
    int (*reset)(struct power_abstract *this) = power_module.ops.reset;
    if (!reset) {
        return HAL_POWER_ENOIMPL;
    }
    return reset(&power_module);
}

int hal_power_resume() {
    if (!power_module.present) {
        return HAL_POWER_ENOFOUND;
    }
    int (*resume)(struct power_abstract *this) = power_module.ops.resume;
    if (!resume) {
        return HAL_POWER_ENOIMPL;
    }
    return resume(&power_module);
}

int hal_power_enter_s1() {
    if (!power_module.present) {
        return HAL_POWER_ENOFOUND;
    }
    int (*enter_s1)(struct power_abstract *this) = power_module.ops.enter_s1;
    if (!enter_s1) {
        return HAL_POWER_ENOIMPL;
    }
    return enter_s1(&power_module);
}

int hal_power_enter_s2() {
    if (!power_module.present) {
        return HAL_POWER_ENOFOUND;
    }
    int (*enter_s2)(struct power_abstract *this) = power_module.ops.enter_s2;
    if (!enter_s2) {
        return HAL_POWER_ENOIMPL;
    }
    return enter_s2(&power_module);
}

int hal_power_enter_s3() {
    if (!power_module.present) {
        return HAL_POWER_ENOFOUND;
    }
    int (*enter_s3)(struct power_abstract *this) = power_module.ops.enter_s3;
    if (!enter_s3) {
        return HAL_POWER_ENOIMPL;
    }
    return enter_s3(&power_module);
}

int hal_power_enter_s4() {
    if (!power_module.present) {
        return HAL_POWER_ENOFOUND;
    }
    int (*enter_s4)(struct power_abstract *this) = power_module.ops.enter_s4;
    if (!enter_s4) {
        return HAL_POWER_ENOIMPL;
    }
    return enter_s4(&power_module);
}

int hal_power_enter_s5() {
    if (!power_module.present) {
        return HAL_POWER_ENOFOUND;
    }
    int (*enter_s5)(struct power_abstract *this) = power_module.ops.enter_s5;
    if (!enter_s5) {
        return HAL_POWER_ENOIMPL;
    }
    return enter_s5(&power_module);
}
