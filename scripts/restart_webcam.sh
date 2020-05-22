#!/bin/bash

# Restarts the USB controller with the webcam and waits until the webcam is
# available again.

exit 0

set -e

while true; do
    # Wait until the webcam is properly closed in case it was just used. When
    # turning the camera off while it is still open, weird things will happen, like
    # the camera getting a different number when it gets detected the next time.
    sleep 0.5

    # Disable the USB controller.
    sudo sh -c "echo 'usb1' > /sys/bus/usb/drivers/usb/unbind"

    # Wait until the webcam is gone. Otherwise it might get a different number after
    # it gets restarted.
    while [ -c /dev/video0 ] || [ -c /dev/video2 ]; do
        sleep 0.01
    done

    sleep 0.2
    sudo sh -c "echo 'usb1' > /sys/bus/usb/drivers/usb/bind"

    # Wait until the webcam is available again.
    until [ -c /dev/video0 ] || [ -c /dev/video2 ]; do
        sleep 0.01
    done

    if [ -c /dev/video0 ]; then
        break
    fi
done
