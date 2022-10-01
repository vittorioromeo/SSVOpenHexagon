#!/bin/bash

set -e

echo ""
echo ""
echo "--------------------------------------------------------------------"
echo "| RUNNING CMAKE IN RELEASE MODE (CLANG + NINJA + TIME-TRACE)       |"
echo "--------------------------------------------------------------------"
echo ""

cmake .. -G"Ninja" \
         -DFORCE_COLORED_OUTPUT=1 \
         -DCMAKE_BUILD_TYPE=RELEASE \
         -DCMAKE_C_COMPILER="clang" \
         -DCMAKE_C_FLAGS="-fuse-ld=lld" \
         -DCMAKE_CXX_COMPILER="clang++" \
         -DCMAKE_CXX_FLAGS="\
             -fuse-ld=lld \
             -Wall -Wextra -Wpedantic -Wno-braced-scalar-init \
             -Wno-pragmas -Wno-missing-field-initializers \
             -O3 -DNDEBUG \
             -frounding-math -ffp-contract=off \
             -ftime-trace \
             -g3"
