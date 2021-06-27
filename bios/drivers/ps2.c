#include <cpu/pio.h>
#include <drivers/ps2.h>

static void poll_read() {
    while (!(inb(PS2_STATUS) & 1));
}

static void poll_write() {
    while (inb(PS2_STATUS) & (1 << 1));
}

static uint8_t keyboard_read() {
    poll_read();
    return inb(PS2_DATA);
}

static void keyboard_write(uint8_t data) {
    poll_write();
    outb(PS2_DATA, data);
}

static uint8_t mouse_read() {
    poll_write();
    outb(PS2_COMMAND, PS2_COMMAND_MOUSE);
    poll_read();
    return inb(PS2_DATA);
}

static void mouse_write(uint8_t data) {
    poll_write();
    outb(PS2_COMMAND, PS2_COMMAND_MOUSE);
    poll_write();
    outb(PS2_DATA, data);
}

int ps2_controller_self_test() {
    poll_write();
    outb(PS2_COMMAND, PS2_COMMAND_TEST);
    poll_read();
    if (inb(PS2_DATA) == 0x55) {
        return 0;
    } else {
        return -1;
    }
}

void ps2_controller_enable_keyb() {
    poll_write();
    outb(PS2_COMMAND, PS2_COMMAND_KEYB_EN);
}

void ps2_controller_disable_keyb() {
    poll_write();
    outb(PS2_COMMAND, PS2_COMMAND_KEYB_DIS);
}

void ps2_controller_enable_mouse() {
    poll_write();
    outb(PS2_COMMAND, PS2_COMMAND_MOUSE_EN);
}

void ps2_controller_disable_mouse() {
    poll_write();
    outb(PS2_COMMAND, PS2_COMMAND_MOUSE_DIS);
}

uint8_t ps2_controller_read_cfg() {
    poll_write();
    outb(PS2_COMMAND, PS2_COMMAND_INT_MEM0_R);
    poll_read();
    return inb(PS2_DATA);
}

void ps2_controller_write_cfg(uint8_t cfg) {
    poll_write();
    outb(PS2_COMMAND, PS2_COMMAND_INT_MEM0_W);
    poll_write();
    outb(PS2_DATA, cfg);
}

void ps2_controller_disable_keyb_irqs() {
    uint8_t cfg = ps2_controller_read_cfg();
    ps2_controller_write_cfg(cfg & ~1);
}

void ps2_controller_disable_mouse_irqs() {
    uint8_t cfg = ps2_controller_read_cfg();
    ps2_controller_write_cfg(cfg & ~(1 << 1));
}

void ps2_controller_disable_keyb_translation() {
    uint8_t cfg = ps2_controller_read_cfg();
    ps2_controller_write_cfg(cfg & ~(1 << 6));
}

void ps2_controller_enable_keyb_irqs() {
    uint8_t cfg = ps2_controller_read_cfg();
    ps2_controller_write_cfg(cfg | 1);
}

void ps2_controller_enable_mouse_irqs() {
    uint8_t cfg = ps2_controller_read_cfg();
    ps2_controller_write_cfg(cfg | (1 << 1));
}

void ps2_controller_enable_keyb_translation() {
    uint8_t cfg = ps2_controller_read_cfg();
    ps2_controller_write_cfg(cfg | (1 << 6));
}

int ps2_keyboard_reset() {
resend:
    keyboard_write(PS2_KEYBOARD_COMMAND_RESET);
    uint8_t data = keyboard_read();
    if (data == 0xfe) {
        goto resend;
    }
    if (data == 0xfa || data == 0xaa) {
        return 0;
    }
    return -1;
}

int ps2_mouse_reset() {
resend:
    mouse_write(PS2_MOUSE_COMMAND_RESET);
    uint8_t data = mouse_read();
    if (data == 0xfe) {
        goto resend;
    }
    if (data == 0xfa || data == 0xaa) {
        return 0;
    }
    return -1;
}
