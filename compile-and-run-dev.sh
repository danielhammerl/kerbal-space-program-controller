#!/bin/bash
g++ -o ksp-controller src/main.cpp -std=c++11 -lkrpc -lprotobuf -lz -O3 -D"KSP_CONTROLLER_DEV_MODE=true" && ./run.sh