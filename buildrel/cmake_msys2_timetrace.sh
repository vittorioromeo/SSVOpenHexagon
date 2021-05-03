#!/bin/bash
cmake .. -G"MinGW Makefiles" -DCMAKE_BUILD_TYPE=DEBUG -DCMAKE_C_COMPILER="clang" -DCMAKE_CXX_COMPILER="clang++" -DCMAKE_CXX_FLAGS="-ftime-trace -Wall -Wextra -Wpedantic -Wno-braced-scalar-init"  
