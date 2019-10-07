#!/bin/bash

# Set the display brightness of the Raspberry Pi touchscreen to a value between 0 and 255. Must be run as root.

brightness="$1"

if [ -z "${brightness}" ]; then
    echo "Usage: ./set_brightness.sh <BRIGHTNESS>"
    exit 1
fi

echo "Set display brightness to ${brightness}"
echo "${brightness}" > /sys/class/backlight/rpi_backlight/brightness
