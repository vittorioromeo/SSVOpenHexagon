#!/bin/bash
cmake .. -G"MinGW Makefiles" -DCMAKE_BUILD_TYPE=DEBUG -DCMAKE_CXX_COMPILER="clang++" -DCMAKE_CXX_FLAGS="-O0 -fno-omit-frame-pointer -Wall -Wextra -Wpedantic -Wno-braced-scalar-init -fsanitize=undefined -fsanitize-undefined-trap-on-error"
