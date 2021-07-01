#ifndef __DRIVERS_PIC_H__
#define __DRIVERS_PIC_H__

#define PIC_MASTER_CMD 0x20
#define PIC_MASTER_DATA 0x21
#define PIC_SLAVE_CMD 0xa0
#define PIC_SLAVE_DATA 0xa1

#define PIC_ICW1_INIT (1 << 4)
#define PIC_ICW4_8086 (1 << 0)

void pic_init(uint8_t master_base, uint8_t slave_base);
void pic_enable_irq(uint8_t irq);
void pic_disable_irq(uint8_t irq);

#endif
