#!/bin/bash

# Restarts the USB controller with the webcam and waits until the webcam is
# available again.

set -e

# Find vendor and product id for the connected webcam.
usb_device=$(lsusb | grep Webcam | cut -d' ' -f 6)
usb_vendor=$(echo $usb_device | cut -d: -f 1)
usb_product=$(echo $usb_device | cut -d: -f 2)

for directory in $(find /sys/bus/usb/devices/ -maxdepth 1 -type l); do
    if [[ -f $directory/idVendor &&
        -f $directory/idProduct &&
        $(cat $directory/idVendor) == $usb_vendor &&
        $(cat $directory/idProduct) == $usb_product ]]; then
        sudo sh -c "echo 0 > $directory/authorized"
        sleep 0.5
        sudo sh -c "echo 1 > $directory/authorized"
    fi
done

# Wait until the webcam is available again. Note that sometimes the camera will
# show up as the wrong device and we cannot do anything against that. This will
# be handled in the software calling this script.
timeout=500
until [ -c /dev/video0 ]; do
    if [ "$timeout" == 0 ]; then
        break
    fi

    sleep 0.01
    ((timeout--))
done
