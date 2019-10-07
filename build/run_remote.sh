#!/bin/bash

# Build the project and run it remotely on a Raspberry Pi.

script_directory="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
cd $script_directory

ssh_host="$1"
if [ -z "${ssh_host}" ]; then
    echo "Usage: ./run_remote.sh <SSH HOST>"
    exit 1
fi

./install_remote.sh "${ssh_host}"

ssh -t "${ssh_host}" "sudo systemctl disable dingdong.service"
ssh -t "${ssh_host}" "sudo systemctl stop dingdong.service"
ssh -t "${ssh_host}" "/opt/dingdong/scripts/launch.sh"
