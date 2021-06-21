#include <print.h>
#include <cpu/pio.h>

void print(const char *msg) {
    while (*msg) {
        outb(0xe9, *msg++);
    }
    outb(0xe9, '\n');
}
