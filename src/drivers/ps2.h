#ifndef __DRIVERS_PS2_H__
#define __DRIVERS_PS2_H__

#include <stddef.h>

#define PS2_DATA 0x60
#define PS2_STATUS 0x64
#define PS2_COMMAND 0x64
#define PS2_KEYB_COMMAND 0x60

int ps2_init();

#endif
