# AtieBIOS
Firmware for the ICH9 chipset. Meant to be ran in emulators, specially QEMU.

# Achievements
- BIOS shadowing/unshadowing by manipulating the DRAM's PAM registers
- Enabling ACPI base register using the LPC
- SMM handling
- PCI enumeration and BAR allocation
- PCI ECAM
- ramfb (a very simple framebuffer interface) 

# Testing
`make all run`
