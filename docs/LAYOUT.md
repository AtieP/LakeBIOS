# Memory layout of LakeBIOS
The memory layout of LakeBIOS is "complex". This is because we have tight space, and because firmware is just a binary blob, it needs to accomodate somehow.  
**Note: the BIOS is really WIP. This can change at any time.**

# Ranges
0x00000-0x8ffff: Usable memory for the operating system.
0x90000-0x9ffff: Heap for permanent data structures. This includes, for example, NVME queues, AHCI command headers/tables...  
0xa0000-0xcffff: The four 64 KB VGA banks. When entering SMM, the first two VGA banks get shadowed and SMRAM appears.  
0xe0000-0xeffff: BIOS data/rodata/bss. These are on their own 64 KB area so they can be exported to RAM, while keeping the BIOS code in ROM, to avoid exploits.  
0xf0000-0xfffff: BIOS code. Here all the BIOS code and drivers are located.

# BIOS code
0xf0000-0xf0fff: The main SMM handler.
0xf1000-0xfdfff: BIOS init, drivers..
0xfe000-0xff000: SMM trampoline. The BIOS relocates this to SMRAM. This bootstraps the SMM handler.
0xff000-0xfffff: BIOS early initialization code. The CPU starts executing code at 0xfffffff0. However, this jumps back to 0xff000 to execute the early initialization code.
