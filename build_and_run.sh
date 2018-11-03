#!/bin/bash

build_directory=build

git fetch --all && git reset --hard origin/master
mkdir -p "${build_directory}"
cd "${build_directory}"
make -j$(nproc)
cd ..

./run.sh

