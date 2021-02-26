#!/bin/bash

script_directory="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
cd $script_directory

docker-compose build dingdong_development_environment
