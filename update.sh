#!/bin/bash

set -e # this makes the script fail on error
git fetch

output=$(git pull)

if [[ $output != *"Already up to date."* ]] || [[ ! -f "ksp-controller" ]]; then
    ./compile.sh
fi