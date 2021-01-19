file build/usbexample.elf
target ext :4242
mon tpiu config internal swout.txt uart off 168000000 1000000
mon adapter srst pulse_width 1
mon adapter srst delay 100
mon reset_config srst_only connect_deassert_srst
set arm force-mode thumb
load


