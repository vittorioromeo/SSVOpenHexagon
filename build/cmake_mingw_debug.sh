#!/bin/bash
cmake .. -G"MinGW Makefiles" -DCMAKE_BUILD_TYPE=DEBUG -DCMAKE_CXX_FLAGS="-O0 -fno-omit-frame-pointer"
