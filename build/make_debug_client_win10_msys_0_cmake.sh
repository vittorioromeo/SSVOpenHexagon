#!/bin/bash

set -e

echo ""
echo ""
echo "--------------------------------------------------------------------"
echo "| RUNNING CMAKE IN DEBUG MODE (NINJA)                              |"
echo "--------------------------------------------------------------------"
echo ""

cmake .. -G"Ninja" \
         -DFORCE_COLORED_OUTPUT=1 \
         -DCMAKE_BUILD_TYPE=DEBUG \
         -DCMAKE_C_COMPILER="gcc" \
         -DCMAKE_C_FLAGS="-fuse-ld=lld" \
         -DCMAKE_CXX_COMPILER="g++" \
         -DCMAKE_CXX_FLAGS="\
            -fuse-ld=lld \
            -O0 -fno-omit-frame-pointer \
            -Wall -Wextra -Wpedantic -Wno-braced-scalar-init -Wno-missing-field-initializers \
            -D_GLIBCXX_ASSERTIONS=1 -D_FORTIFY_SOURCE=2 \
            -fstack-protector -Wno-pragmas\
            -frounding-math -fsignaling-nans -ffloat-store -ffp-contract=off"

