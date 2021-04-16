#!/bin/bash
cmake .. -DCMAKE_CXX_COMPILER=g++ -DCMAKE_BUILD_TYPE=RELEASE -DCMAKE_CXX_FLAGS="-Wno-deprecated-declarations -Wall -Wextra -Wpedantic"


