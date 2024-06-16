#!/bin/bash
cmake .. -G"MinGW Makefiles" -DCMAKE_BUILD_TYPE=DEBUG -DCMAKE_C_COMPILER="gcc" -DCMAKE_CXX_COMPILER="g++" -DCMAKE_CXX_FLAGS="-O0 -fno-omit-frame-pointer -Wall -Wextra -Wpedantic -Wno-braced-scalar-init -D_GLIBCXX_ASSERTIONS=1  -D_FORTIFY_SOURCE=3 -fstack-protector -Wno-pragmas"
