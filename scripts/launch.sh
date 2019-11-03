#!/bin/bash

# Launch script for the dingdong GUI. This script will set up various things that are needed for running dingdong
# before starting the GUI. Note that the script needs to be run as the user whose X and PulseAudio session should be
# used. The user needs to be able to configure some system services using sudo.

set -e

DINGDONG_PATH="/opt/dingdong"
cd "${DINGDONG_PATH}"

# Power saving.
# =============

# Disable HDMI output on Raspberry Pi 4, which can generate a lot of heat. The 4K output also automatically raises the
# clock of the GPU. See https://www.raspberrypi.org/documentation/configuration/config-txt/overclocking.md for more
# information.
# Note that this command will fail on earlier versions of the Raspberry Pi, but those don't have the critical 4K HDMI
# output anyway.
sudo raspi-config nonint do_pi4video V3 || true

# GPIO setup.
# ===========

# Enable SPI for the RFID reader (0 means on).
sudo raspi-config nonint do_spi 0

# Enable I2C for the relay shield (0 means on).
sudo raspi-config nonint do_i2c 0

# Screen setup.
# =============

sudo ./scripts/display/set_brightness.sh 100
./scripts/display/set_screensaver_timeout.sh 20
./scripts/display/force_on.sh

# Audio Setup.
# ============

# It is necessary to export this variable to connect to the user's PulseAudio session from within a systemd service.
export PULSE_RUNTIME_PATH="/run/user/$(id -u)/pulse/"

if [ ! -d "${PULSE_RUNTIME_PATH}" ]; then
    echo "PulseAudio user session not running yet"
    exit 1
fi

./scripts/audio/choose_audio_source.sh
./scripts/audio/set_volume.sh

# Ensure the echo cancellation module is loaded for the audio source that was chosen above. This creates a virtual sink
# and source. Make sure that we use those devices, otherwise echo cancellation might not work.
pactl unload-module module-echo-cancel
pactl load-module module-echo-cancel source_name=noechosource sink_name=noechosink aec_method=webrtc aec_args="analog_gain_control=0\ digital_gain_control=1"
pacmd set-default-source "noechosource"
pacmd set-default-sink "noechosink"

# Launch the dingdong GUI.
# ========================

# The PULSE_PROP environment variable enables PulseAudio echo cancellation.
DISPLAY=:0 PULSE_PROP=\"filter.want=echo-cancel\" ./bin/dingdong
