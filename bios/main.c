#include <stdint.h>

__attribute__((__section__(".c_init"), used))
void bios_main() {
    char *string = "Hello from C code!";
    while (*string) {
        asm volatile("out %0, %1" :: "a"((uint8_t) *string++), "Nd"((uint16_t) 0xe9));
    }
    for (;;);
}
