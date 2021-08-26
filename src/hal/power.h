#ifndef __HAL_POWER_H__
#define __HAL_POWER_H__

#define HAL_POWER_I440FX 1
#define HAL_POWER_Q35    2

#define HAL_POWER_S0 0
#define HAL_POWER_S1 1
#define HAL_POWER_S2 2
#define HAL_POWER_S3 3
#define HAL_POWER_S4 4
#define HAL_POWER_S5

struct power_abstract {
    int present;
    int interface;
    int last_state;
    struct {
        int (*reset)(struct power_abstract *this);
        int (*resume)(struct power_abstract *this);
        int (*enter_s1)(struct power_abstract *this);
        int (*enter_s2)(struct power_abstract *this);
        int (*enter_s3)(struct power_abstract *this);
        int (*enter_s4)(struct power_abstract *this);
        int (*enter_s5)(struct power_abstract *this);
    } ops;
};

#define HAL_POWER_ESUCCESS  0
#define HAL_POWER_ENOFOUND -1
#define HAL_POWER_ENOIMPL  -2
#define HAL_POWER_EUNK     -3

int hal_power_submit(struct power_abstract *power_abstract);

int hal_power_reset();
int hal_power_resume();

int hal_power_enter_s1();
int hal_power_enter_s2();
int hal_power_enter_s3();
int hal_power_enter_s4();
int hal_power_enter_s5();

#endif
