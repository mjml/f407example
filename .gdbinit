file build/usbexample.elf
target ext :4242
monitor tpiu config internal swout.txt uart off 168000000 1000000
set arm force-mode thumb
load


