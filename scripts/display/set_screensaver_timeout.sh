#!/bin/bash

# Set the screensaver timeout in seconds.

timeout="$1"

if [ -z "${timeout}" ]; then
    echo "Usage: ./set_screensaver_timeout.sh <TIMEOUT>"
    exit 1
fi

DISPLAY=:0 xset dpms "${timeout}" "${timeout}" "${timeout}"
