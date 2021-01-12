#!/bin/bash

# Which USB device do we pass through?
address=$(lsusb | awk -e '/Hermit\x27s/ { print gensub(/\y0*/, "", "g", $2 " " gensub(/:$/, "", "g", $4)) }' | sort -k 2 | tail -1)
busnum=$(echo $address | cut -f 1 -d \  -- )
devnum=$(echo $address | cut -f 2 -d \  -- )

echo USB Device scan found:
echo address=$address
echo busnum=$busnum, devnum=$devnum

# Tell virsh about the USB device
cat <<EOF > virt-usb.xml
  <hostdev mode='subsystem' type='usb'>
    <source startupPolicy='optional'>
      <address bus='$busnum' device='$devnum'/>
    </source>
  </hostdev>
EOF

virsh attach-device kdev virt-usb.xml --config

# Start the actual VM
virsh start kdev --console

# After it's done, detach the device
virsh detach-device kdev virt-usb.xml --config
