#!/bin/bash
cmake .. -G"Ninja" -DCMAKE_BUILD_TYPE=DEBUG -DCMAKE_CXX_FLAGS="-O0 -fno-omit-frame-pointer -Wall -Wextra -Wpedantic" -DCMAKE_SYSTEM_NAME="Windows" "$@"
