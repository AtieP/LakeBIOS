#ifndef __APIS_BIOS_BDA_H__
#define __APIS_BIOS_BDA_H__

#include <stdint.h>

#define APIS_BIOS_BDA 0x400

void api_bios_bda_com_set(int com, uint16_t addr);
void api_bios_bda_lpt_set(int lpt, uint16_t addr);
uint16_t api_bios_bda_com_get(int com);
uint16_t api_bios_bda_lpt_get(int lpt);

void api_bios_bda_floppy_status_set(uint8_t status);
void api_bios_bda_disk_status_set(uint8_t status);
uint8_t api_bios_bda_floppy_status_get();
uint8_t api_bios_bda_disk_status_get();

void api_bios_bda_update_keyb_buffer(uint8_t scancode);

#endif
