#include <stddef.h>
#include <stdint.h>
#include <cpu/misc.h>
#include <cpu/pio.h>
#include <drivers/hid/ps2.h>
#include <tools/print.h>

// Before reading data from port 0x60
static void wait_read() {
    while (!(inb(PS2_STATUS) & 1)) {
        pause();
    }
}

// Before writing data to ports 0x60 or 0x64
static void wait_write() {
    while (inb(PS2_STATUS) & (1 << 1)) {
        pause();
    }
}

static void flush_buffer() {
    while (inb(PS2_STATUS) & 1) {
        inb(PS2_DATA);
    }
}

// Output can be NULL to read spare bytes
static int controller_command(uint8_t command, uint8_t *input, int input_len, uint8_t *output, int output_len) {
    wait_write();
    outb(PS2_COMMAND, command);
    for (int i = 0; i < input_len; i++) {
        wait_write();
        outb(PS2_DATA, input[i]);
    }
    for (int i = 0; i < output_len; i++) {
        wait_read();
        uint8_t val = inb(PS2_DATA);
        if (output) {
            output[i] = val;
        }
    }
    return 0;
}

static int keyboard_command(uint8_t command, uint8_t *input, int input_len, uint8_t *output, int output_len) {
    static int tries;
    tries = 0;
send:
    wait_write();
    outb(PS2_KEYB_COMMAND, command);
    for (int i = 0; i < input_len; i++) {
        wait_write();
        outb(PS2_KEYB_COMMAND, input[i]);
    }
    int acknowledged = 0;
    for (int i = 0; i < output_len; i++) {
read:
        wait_read();
        uint8_t val = inb(PS2_DATA);
        if (i == 0) {
            // Does it need to be resent?
            if (val == 0xfe) {
                if (tries == 3) {
                    print("PS/2: Failed to deliver the command to the keyboard after 3 tries");
                    return -1;
                }
                tries++;
                goto send;
            }
            // Did it acknowledge or echo?
            if (!acknowledged) {
                if (val == 0xfa) {
                    acknowledged = 1;
                    goto read;
                } else if (val == 0xee && command == 0xee) {
                    acknowledged = 1;
                    goto read;
                } else {
                    // Assume the keyboard is faulty and try again maybe
                    if (tries == 3) {
                        print("PS/2: Failed to deliver the command to the keyboard after 3 tries");
                        return -1;
                    }
                    tries++;
                    goto send;
                }
            }
        }
        if (output) {
            output[i] = val;
        }
    }
    return 0;
}

static int ps2_controller_init() {
    uint8_t ret;
    controller_command(0xaa, NULL, 0, &ret, 1);
    if (ret != 0x55) {
        print("PS/2: Controller self test failed");
        return -1;
    }
    return 0;
}

static int ps2_keyboard_init() {
    // How are the clock/data lines?
    uint8_t ret;
    controller_command(0xab, NULL, 0, &ret, 1);
    if (ret == 0x01) {
        print("PS/2: Keyboard interface clock line stuck low");
        return -1;
    } else if (ret == 0x02) {
        print("PS/2: Keyboard interface clock line stuck high");
        return -1;
    } else if (ret == 0x03) {
        print("PS/2: Keyboard interface data line stuck low");
        return -1;
    } else if (ret != 0x00) {
        print("PS/2: Keyboard interface status unknown");
        return -1;
    }
    // Enable interface
    controller_command(0xae, NULL, 0, NULL, 0);
    // Reset
    if (keyboard_command(0xff, NULL, 0, &ret, 1) != 0 || ret != 0xaa) {
        print("PS/2: Keyboard self-test failed");
    }
    // Start scanning
    if (keyboard_command(0xf4, NULL, 0, NULL, 0) != 0) {
        print("PS/2: Keuboard start scanning failure");
    }
    return 0;
}

int ps2_init() {
    int ret = 0;
    // Disable keyboard and mouse interfaces
    controller_command(0xad, NULL, 0, NULL, 0);
    controller_command(0xa7, NULL, 0, NULL, 0);
    flush_buffer();
    if (ps2_controller_init() != 0) {
        print("PS/2: Could not initialize the controller");
        ret |= 1;
        return ret;
    }
    if (ps2_keyboard_init() != 0) {
        print("PS/2: Could not initialize the keyboard");
        ret |= (1 << 1);
    }
    // TODO: Mouse
    return ret;
}
