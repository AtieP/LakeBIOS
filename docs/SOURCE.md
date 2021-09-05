# Source code organization of LakeBIOS

# docs/
Documentation about LakeBIOS.

# src/
Main BIOS source code.

* src/apis: Exposable to operating systems functions.
* src/cpu: CPU related routines.
* src/drivers: Drivers for hardware that do not depend on chipset specific features, like ordinary PCI features, disk controllers, video cards, interrupt controllers, input devices... Separated in categories.
* src/hal: Abstract glue for hardware.
* src/misc: For files that do not have a clear folder destination, like compiler specific files.
* src/motherboard: Motherboard specific code. Motherboard specific hardware goes to that folder too. Classified by vendor/motherboard variant.
* src/tools: Utilities, like allocators, loggers, math functions...
