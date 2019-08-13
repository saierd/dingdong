#!/bin/bash

# Run the given command (or an interactive bash instance) in a development environment Docker container.

script_directory="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
cd $script_directory

docker-compose build dingdong_development_environment
if [ $# -eq 0 ]; then
    docker-compose run --rm dingdong_development_environment bash
else
    docker-compose run --rm dingdong_development_environment bash -c "$@"
fi
