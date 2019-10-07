#!/bin/bash

# Turn on the display when the screensaver currently has it turned off.
DISPLAY=:0 xset dpms force on

# Reset idle time.
DISPLAY=:0 xset s reset
