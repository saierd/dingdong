#!/bin/bash

# Launch script for the dingdong GUI. This script will set up various things that are needed for running dingdong
# before starting the GUI. Note that the script needs to be run as the user whose X and PulseAudio session should be
# used. The user needs to be able to configure some system services using sudo.

set -e

DINGDONG_PATH="/opt/dingdong"
cd "${DINGDONG_PATH}"

# Enable the pigpio daemon. We use the pigpio client library to connect to this daemon and control the GPIOs.
sudo systemctl enable pigpiod
sudo systemctl start pigpiod

# Enable SPI for the RFID reader (0 means on).
sudo raspi-config nonint do_spi 0

# Reduce the screen brightness.
sudo ./scripts/display/set_brightness.sh 100

# Audio Setup.
# It is necessary to export this variable to connect to the user's PulseAudio session from within a systemd service.
export PULSE_RUNTIME_PATH="/run/user/$(id -u)/pulse/"

if [ ! -d "${PULSE_RUNTIME_PATH}" ]; then
    echo "PulseAudio user session not running yet"
    exit 1
fi

./scripts/audio/choose_audio_source.sh
./scripts/audio/set_volume.sh

# Launch the dingdong GUI.
# The PULSE_PROP environment variable enables PulseAudio echo cancellation.
DISPLAY=:0 PULSE_PROP=\"filter.want=echo-cancel\" ./bin/dingdong
