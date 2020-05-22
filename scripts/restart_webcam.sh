#!/bin/bash

# Restarts the USB controller with the webcam and waits until the webcam is
# available again.

set -e

# Wait until the webcam is properly closed in case it was just used.
#sleep 0.5

# Find vendor and product id for the connected webcam.
usb_device=$(lsusb | grep Webcam | cut -d' ' -f 6)
usb_vendor=$(echo $usb_device | cut -d: -f 1)
usb_product=$(echo $usb_device | cut -d: -f 2)

#while true; do
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

    # Wait until the webcam is available again.
    timeout=500
    until [ -c /dev/video0 ]; do
        if [ "$timeout" == 0 ]; then
            break
        fi

        sleep 0.01
        ((timeout--))
    done

    # Sometimes the webcam will show up as a different video device. In that
    # case we restart it until it shows up as /dev/video0 again.
    #if [ -c /dev/video0 ]; then
    #    sleep 0.1
    #    break
    #fi
#done

exit 0



while true; do
    # Wait until the webcam is properly closed in case it was just used. When
    # turning the camera off while it is still open, weird things will happen, like
    # the camera getting a different number when it gets detected the next time.
    sleep 1

    # Disable the USB controller.
    sudo sh -c "echo 'usb1' > /sys/bus/usb/drivers/usb/unbind" || true

    # Wait until the webcam is gone. Otherwise it might get a different number after
    # it gets restarted.
    while [ -c /dev/video0 ] || [ -c /dev/video2 ]; do
        sleep 0.01
    done

    sleep 0.5
    sudo sh -c "echo 'usb1' > /sys/bus/usb/drivers/usb/bind" || true

    # Wait until the webcam is available again.
    timeout=500
    until [ -c /dev/video0 ] || [ -c /dev/video2 ]; do
        if [ "$timeout" == 0 ]; then
            break
        fi

        sleep 0.01
        ((timeout--))
    done

    if [ -c /dev/video0 ]; then
        sleep 0.1
        break
    fi
done
