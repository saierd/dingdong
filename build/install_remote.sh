#!/bin/bash

# Build the project and install it remotely on a Raspberry Pi.

set -e

script_directory="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
cd $script_directory

ssh_host="$1"
if [ -z "${ssh_host}" ]; then
    echo "Usage: ./install_remote.sh <SSH HOST>"
    exit 1
fi

remote_package_directory="/tmp/dingdong_packages"

./build.sh

cd ../build-raspberry
ssh -t "${ssh_host}" "rm -rf ${remote_package_directory} && mkdir -p ${remote_package_directory}"
scp *.deb "${ssh_host}:${remote_package_directory}"
# Installation might fail when dependencies are missing. This will be fixed below.
ssh -t "${ssh_host}" "sudo dpkg --force-confold --force-confdef --force-confmiss -i ${remote_package_directory}/*.deb" || true
ssh -t "${ssh_host}" "sudo apt-get install -y --fix-broken"
