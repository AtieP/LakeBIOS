#include <cpu/pio.h>
#include <drivers/pic.h>
#include <tools/print.h>

void pic_init(uint8_t master_base, uint8_t slave_base) {
    // (ICW1) Tell master and slave PIC what we're initializing them
    outb(PIC_MASTER_CMD, PIC_ICW1_INIT);
    outb(PIC_SLAVE_CMD, PIC_ICW1_INIT);
    // (ICW2) Tell master and slave PIC their bases
    outb(PIC_MASTER_DATA, master_base);
    outb(PIC_SLAVE_DATA, slave_base);
    // (ICW3) Tell master and slave PIC that they're cascaded to IRQ 2
    outb(PIC_MASTER_DATA, 4);
    outb(PIC_SLAVE_DATA, 2);
    // (ICW4) 8086 mode
    outb(PIC_MASTER_DATA, PIC_ICW4_8086);
    outb(PIC_SLAVE_DATA, PIC_ICW4_8086);
    // Mask all IRQs
    outb(PIC_MASTER_DATA, 0xff);
    outb(PIC_SLAVE_DATA, 0xff);
}

void pic_enable_irq(uint8_t irq) {
    if (irq > 15) {
        print("PIC: IRQ out of bounds (%d)", irq);
        return;
    }
    if (irq > 7) {
        irq -= 8;
        outb(PIC_SLAVE_DATA, inb(PIC_SLAVE_DATA) & ~(1 << irq));
    } else {
        outb(PIC_MASTER_DATA, inb(PIC_MASTER_DATA) & ~(1 << irq));
    }
}

void pic_disable_irq(uint8_t irq) {
    if (irq > 15) {
        print("PIC: IRQ out of bounds (%d)", irq);
        return;
    }
    if (irq > 7) {
        irq -= 8;
        outb(PIC_SLAVE_DATA, inb(PIC_SLAVE_DATA) | (1 << irq));
    } else {
        outb(PIC_MASTER_DATA, inb(PIC_MASTER_DATA) | (1 << irq));
    }
}

void pic_set_level(uint8_t irq) {
    if (irq > 15) {
        print("PIC: IRQ out of bounds (%d)", irq);
        return;
    }
    if (irq > 7) {
        irq -= 8;
        outb(PIC_SLAVE_ELCR, inb(PIC_SLAVE_ELCR) | (1 << irq));
    } else {
        outb(PIC_MASTER_ELCR, inb(PIC_MASTER_ELCR) | (1 << irq));
    }
}

void pic_set_edge(uint8_t irq) {
    if (irq > 15) {
        print("PIC: IRQ out of bounds (%d)", irq);
        return;
    }
    if (irq > 7) {
        irq -= 8;
        outb(PIC_SLAVE_ELCR, inb(PIC_SLAVE_ELCR) & ~(1 << irq));
    } else {
        outb(PIC_MASTER_ELCR, inb(PIC_MASTER_ELCR) & ~(1 << irq));
    }
}
