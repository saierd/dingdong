#!/bin/bash

apt-get install -y libgstreamer1.0-dev libgtkmm-3.0-dev uuid-dev

hardware_name=$(uname -m)
if [ "${hardware_name}" == "armv7l" ]; then
    # Raspbian currently has a pretty old version of CMake. Install a current one from source.
    apt remove -y cmake
    apt purge -y --auto-remove cmake

    cd /tmp
    wget https://cmake.org/files/v3.12/cmake-3.12.1.tar.gz -O cmake.tar.gz
    tar -xvzf cmake.tar.gz
    cd cmake-3.12.1
    ./bootstrap
    make
    make install
fi

