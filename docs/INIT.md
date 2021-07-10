# Initialization procedure
(Please read LAYOUT.md first).

1. The CPU starts executing code at 0xfffffff0. The BIOS jumps to 0xff000 and bootstraps the BIOS, to later jump to the main BIOS code.
2. The BIOS detects the chipset that it's running on. Depending on it, it does chipset specific initialization, like PCI BAR allocation, pin assigning, ACPI base register setting, SMRAM setup...
3. The BIOS initializes all other devices.
