#include <stdarg.h>
#include <print.h>
#include <string.h>
#include <cpu/pio.h>

static void puts(const char *msg) {
    while (*msg) {
        outb(0xe9, *msg++);
    }
}

void print(const char *msg, ...) {
    va_list args;
    va_start(args, msg);
    while (*msg) {
        if (*msg == '%') {
            msg++;
            if (*msg == 'd') {
                char number_str[11];
                int number = va_arg(args, int);
                memset(&number_str, 0, 11);
                for (int i = 10; i > 0;) {
                    number_str[--i] = number % 10 + '0';
                    number /= 10;
                }
                puts(number_str);
            }
        } else {
            outb(0xe9, *msg);
        }
        msg++;
    }
    outb(0xe9, '\n');
    va_end(args);
}
