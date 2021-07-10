#include <cpu/idt.h>

static struct idt_entry idt[256];

__attribute__((__section__(".int_main_code"), __used__))
static void int_handler_main() {
    for (;;) {}
}

void idt_init() {

}
