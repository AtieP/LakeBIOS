#ifndef __HAL_POWER_H__
#define __HAL_POWER_H__

#define HAL_POWER_I440FX 1
#define HAL_POWER_Q35    2

struct power_abstract {
    int present;
    int interface;
    struct {
        int (*reset)(struct power_abstract *this);
        int (*resume)(struct power_abstract *this);
        int (*s1)(struct power_abstract *this);
        int (*s2)(struct power_abstract *this);
        int (*s3)(struct power_abstract *this);
        int (*s4)(struct power_abstract *this);
        int (*s5)(struct power_abstract *this);
    } ops;
};

#define HAL_POWER_ESUCCESS  0
#define HAL_POWER_ENOFOUND -1
#define HAL_POWER_ENOIMPL  -2
#define HAL_POWER_EUNK     -3

int hal_power_submit(struct power_abstract *power_abstract);

int hal_power_reset();
int hal_power_resume();

int hal_power_s1();
int hal_power_s2();
int hal_power_s3();
int hal_power_s4();
int hal_power_s5();

#endif
