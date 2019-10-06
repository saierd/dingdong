#!/bin/bash

# Set output volume to the maximum. The actual volume can be adjusted using the amplifier of the speaker.
echo "Set default audio volume to maximum"
pactl set-sink-volume alsa_output.platform-soc_audio.analog-mono 100%
