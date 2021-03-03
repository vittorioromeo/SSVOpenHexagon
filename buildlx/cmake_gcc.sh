#!/bin/bash
cmake .. -DCMAKE_CXX_COMPILER=g++ -DCMAKE_BUILD_TYPE=DEBUG -DCMAKE_CXX_FLAGS="-Wno-deprecated-declarations -g -fsanitize=undefined -Wall -Wextra -Wpedantic"


