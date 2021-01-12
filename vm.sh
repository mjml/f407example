#!/bin/bash

# Which USB device do we pass through?
address=$(lsusb | grep "Hermit's" | awk -e '{print $2 " " $4 }' | sed -e 's/\:$//' | sed -e 's/\b0*//g')
busnum=$(echo $address | cut -f 1 -d \  -- )
devnum=$(echo $address | cut -f 2 -d \  -- )

echo address = $address
echo busnum = $busnum
echo devnum = $devnum

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
