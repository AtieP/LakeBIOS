#include <stdarg.h>
#include <cpu/pio.h>
#include <tools/print.h>
#include <tools/string.h>

static void puts(const char *msg) {
    while (*msg) {
        outb(0xe9, *msg++);
    }
}

void print(const char *msg, ...) {
    puts("lakebios: ");
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
                char *number_str_ptr = (char *) &number_str;
                int counter = 0;
                while (number_str_ptr[counter] == '0' && counter < 9) {
                    counter++;
                }
                puts(&number_str_ptr[counter]);
            }
            if (*msg == 'x') {
                char number_str[8];
                int number = va_arg(args, int);
                memset(&number_str, 0, 9);
                for (int i = 8; i > 0;) {
                    number_str[--i] = "0123456789abcdef"[number & 0x0f];
                    number >>= 4;
                }
                char *number_str_ptr = (char *) &number_str;
                int counter = 0;
                while (number_str_ptr[counter] == '0' && counter < 6) {
                    counter++;
                }
                puts(&number_str_ptr[counter]);
            }
            if (*msg == 's') {
                const char *string = va_arg(args, const char *);
                puts(string);
            }
            if (*msg == 'S') {
                const char *string = va_arg(args, const char *);
                int length = va_arg(args, int);
                for (int i = 0; i < length; i++) {
                    outb(0xe9, string[i]);
                }
            }
            if (*msg == 'c') {
                int ch = va_arg(args, int);
                outb(0xe9, ch);
            }
        } else {
            outb(0xe9, *msg);
        }
        msg++;
    }
    outb(0xe9, '\n');
    va_end(args);
}
