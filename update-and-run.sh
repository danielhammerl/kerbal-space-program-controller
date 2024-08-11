#!/bin/bash

git fetch

output=$(git pull)

if [[ $output != *"Already up to date."* ]] || [[ ! -f "ksp-controller" ]]; then
    ./compile.sh
fi

./run.sh