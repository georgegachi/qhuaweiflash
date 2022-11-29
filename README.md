# qhuaweiflash

A graphical utility for flashing HUAWEI modems and routers and editing firmware files

This utility is designed for:

- Firmware for huawei modems that support the firmware protocol similar to that used in modems on Balong V7. Including implemented full-fledged work with digital signatures of firmware.

- Editing firmware images. You can view, add, delete, change individual sections, change section headings. Implemented editing of partition images in HEX code and, partially, in format mode (if the partition has any meaningful format).
- Downloads to the usbloader modem using patches.

The utility is built on the Qt graphics package, and is a windowed version of the balong_flash, balong-usbload utilities, and also a firmware editor.

To build the utility, use the commands:

qmake

make