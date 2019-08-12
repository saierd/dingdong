#!/bin/bash

# Cross-compile the project for Raspberry Pi.

build_directory="build-raspberry"

script_directory="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
cd $script_directory/docker

./run.sh "mkdir -p ${build_directory}"
./run.sh "cd ${build_directory} && cmake -GNinja -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=../build/toolchain.cmake .. && cmake --build ."
