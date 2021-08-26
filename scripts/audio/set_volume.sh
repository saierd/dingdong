#!/bin/bash

# Set output volume to the maximum. The actual volume can be adjusted using the amplifier of the speaker.
echo "Set default audio volume to maximum"

device_name="alsa_output.platform-bcm2835_audio.analog-mono"

pactl list | grep "alsa_output.platform-soc_audio.analog-mono" &> /dev/null
if [ $? == 0 ]; then
    device_name="alsa_output.platform-soc_audio.analog-mono"
fi

pactl list | grep "alsa_output.platform-bcm2835_audio.analog-stereo" &> /dev/null
if [ $? == 0 ]; then
    device_name="alsa_output.platform-bcm2835_audio.analog-stereo"
fi

pactl set-sink-volume "${device_name}" 100%
