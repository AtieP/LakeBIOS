# AtieBIOS
Firmware for the ICH9 chipset. Meant to be ran in emulators, specially QEMU.

# Achievements
- BIOS shadowing/unshadowing by manipulating the DRAM's PAM registers
- Enabling ACPI base register using the LPC
- SMM handling
- PCI enumeration and BAR allocation
- PCI ECAM
- ramfb (a very simple framebuffer interface) 
- AHCI
- NVME

# Testing
`make all run`

# Attribution
Wallpaper at the root folder of the project made by chienba. https://pixabay.com/photos/grass-plants-flowers-garden-flora-6353411/