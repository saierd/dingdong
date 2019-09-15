#!/bin/bash

sudo apt-get update
sudo apt-get -y install --no-install-recommends \
    gstreamer1.0-pulseaudio \
    libgtkmm-3.0-1v5 \
    pulseaudio \
    sox

# Enable the pigpio daemon. We use the pigpio client library to connect to this daemon and control the GPIOs.
sudo systemctl enable pigpiod
sudo systemctl start pigpiod

# Enable SPI for the RFID reader (0 means on).
sudo raspi-config nonint do_spi 0

# Install a Python library for the RFID reader.
sudo pip3 install spidev pi-rc522
