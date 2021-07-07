#ifndef __DRIVERS_FLOPPY_H__
#define __DRIVERS_FLOPPY_H__

#include <stdint.h>

#define FLOPPY_1440_CYL 80
#define FLOPPY_1440_HDS 2
#define FLOPPY_1440_SEC 18

#define FLOPPY_BASE_MASTER 0x3f0
#define FLOPPY_BASE_SLAVE 0x370

#define FLOPPY_SRA 0x00
#define FLOPPY_SRB 0x01
#define FLOPPY_DOR 0x02
#define FLOPPY_TDR 0x03
#define FLOPPY_MSR 0x04
#define FLOPPY_DSR 0x04
#define FLOPPY_DF  0x05
#define FLOPPY_DIR 0x07
#define FLOPPY_CCR 0x07

#define FLOPPY_DOR_MOTD (1 << 7)
#define FLOPPY_DOR_MOTC (1 << 6)
#define FLOPPY_DOR_MOTB (1 << 5)
#define FLOPPY_DOR_MOTA (1 << 4)
#define FLOPPY_DOR_IRQ_DMA (1 << 3)
#define FLOPPY_DOR_RESET (1 << 2)
#define FLOPPY_DOR_DSEL1 (1 << 1)
#define FLOPPY_DIR_DSEL2 (1 << 0)

#define FLOPPY_MSR_RQM (1 << 7)
#define FLOPPY_MSR_DIO (1 << 6)
#define FLOPPY_MSR_NDMA (1 << 5)
#define FLOPPY_MSR_CB (1 << 4)
#define FLOPPY_MSR_ACTD (1 << 3)
#define FLOPPY_MSR_ACTC (1 << 2)
#define FLOPPY_MSR_ACTB (1 << 1)
#define FLOPPY_MSR_ACTA (1 << 0)

#define FLOPPY_CMD_VERSION 16

void floppy_init();
int floppy_command(uint16_t io_base, uint8_t command, uint8_t *input, int input_len, uint8_t *output, int output_len, int skip_exec_state);

#endif
