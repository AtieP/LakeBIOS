#ifndef __DRIVERS_PS2_H__
#define __DRIVERS_PS2_H__

#define PS2_DATA 0x60
#define PS2_STATUS 0x64
#define PS2_COMMAND 0x64

#define PS2_COMMAND_INT_MEM0_R 0x20
#define PS2_COMMAND_INT_MEM0_W 0x60

#define PS2_COMMAND_TEST 0xaa
#define PS2_COMMAND_KEYB_TEST 0xab
#define PS2_COMMAND_MOUSE_TEST 0xa9

#define PS2_COMMAND_KEYB_EN 0xae
#define PS2_COMMAND_KEYB_DIS 0xad
#define PS2_COMMAND_MOUSE_EN 0xa8
#define PS2_COMMAND_MOUSE_DIS 0xa7
#define PS2_COMMAND_MOUSE 0xd4

#define PS2_KEYBOARD_COMMAND_RESET 0xff
#define PS2_KEYBOARD_COMMAND_SCANNING_EN 0xf4
#define PS2_KEYBOARD_COMMAND_SCANNING_DIS 0xf5

#define PS2_MOUSE_COMMAND_RESET 0xff

int ps2_controller_self_test();
void ps2_controller_enable_keyb_port();
void ps2_controller_disable_keyb_port();
void ps2_controller_enable_mouse_port();
void ps2_controller_disable_mouse_port();
uint8_t ps2_controller_read_cfg();
void ps2_controller_write_cfg(uint8_t cfg);

void ps2_controller_disable_keyb_irqs();
void ps2_controller_disable_mouse_irqs();
void ps2_controller_disable_keyb_translation();
void ps2_controller_enable_keyb_irqs();
void ps2_controller_enable_mouse_irqs();
void ps2_controller_enable_keyb_translation();

int ps2_keyboard_reset();
void ps2_keyboard_enable_scanning();
void ps2_keyboard_disable_scanning();

int ps2_mouse_reset();

#endif
