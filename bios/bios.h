#ifndef __BIOS_H__
#define __BIOS_H__

#define BDA_ADDRESS 0x400
#define BDA_SIZE 256

#define LOWMEM_END 0xa0000

#define EBDA_SIZE 0x2000
#define EBDA_ADDRESS (LOWMEM_END - EBDA_SIZE)

#endif
