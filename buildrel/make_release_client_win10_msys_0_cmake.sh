#!/bin/bash

set -e

echo ""
echo ""
echo "--------------------------------------------------------------------"
echo "| RUNNING CMAKE IN RELEASE MODE (MINGW MAKEFILES)                  |"
echo "--------------------------------------------------------------------"
cmake .. -G"MinGW Makefiles" \
         -DCMAKE_BUILD_TYPE=RELEASE \
         -DCMAKE_C_COMPILER="gcc" \
         -DCMAKE_CXX_COMPILER="g++" \
         -DCMAKE_CXX_FLAGS="\
             -Wall -Wextra -Wpedantic -Wno-braced-scalar-init \
             -Wno-pragmas -Wno-missing-field-initializers \
             -O3 -DNDEBUG"
