#!/bin/bash

virsh detach-device kdev virt-usb.xml --live
sleep 2
virsh attach-device kdev virt-usb.xml --live

