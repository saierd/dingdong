#!/bin/bash

# Build the project and run it remotely on a Raspberry Pi.

script_directory="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
cd $script_directory

ssh_host="$1"
if [ -z "${ssh_host}" ]; then
    echo "Usage: ./run_remote.sh <SSH HOST>"
    exit 1
fi

./build.sh

cd ../build-raspberry
scp dingdong "${ssh_host}:dingdong"
ssh -t "${ssh_host}" "DISPLAY=:0 ./dingdong"
