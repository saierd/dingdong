#!/bin/bash

# Sets the PulseAudio default source to the USB microphone if one is found.

# A filter for choosing the desired audio source from the list of sources. If there are multiple
# devices matching this text, we will use the first one.
device_filter="usb"

device_name=$(pacmd list-sources | grep "name:" | grep "${device_filter}" | head -n 1 | sed 's/^\s*name:\s*<\(.*\)>/\1/')

if [ ! -z "${device_name}" ]; then
    echo "Set default audio source to '${device_name}'"
    pacmd set-default-source "${device_name}"
else
    echo "COULD NOT FIND A SUITABLE AUDIO SOURCE!"
    exit 1
fi
