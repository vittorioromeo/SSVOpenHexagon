#!/bin/bash

set -e

echo ""
echo ""
echo "--------------------------------------------------------------------"
echo "| RUNNING CMAKE IN RELEASE MODE (NINJA)                            |"
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
             -Og -g3 -fno-omit-frame-pointer \
             -Wall -Wextra -Wpedantic -Wno-braced-scalar-init \
             -D_GLIBCXX_ASSERTIONS=1 -D_FORTIFY_SOURCE=2 \
             -fstack-protector -Wno-pragmas \
             -Wno-pragmas -Wno-missing-field-initializers \
             -DSSVOH_HEADLESS_TESTS=1 \
             -frounding-math -fsignaling-nans -ffloat-store -ffp-contract=off"

