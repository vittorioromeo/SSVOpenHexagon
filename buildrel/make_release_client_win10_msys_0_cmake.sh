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
         -DCMAKE_BUILD_TYPE=RELEASE \
         -DCMAKE_C_COMPILER="gcc" \
         -DCMAKE_C_FLAGS="-fuse-ld=lld" \
         -DCMAKE_CXX_COMPILER="g++" \
         -DCMAKE_CXX_FLAGS="\
             -fuse-ld=lld \
             -Wall -Wextra -Wpedantic -Wno-braced-scalar-init \
             -Wno-pragmas -Wno-missing-field-initializers \
             -Wno-stringop-overflow \
             -O3 -DNDEBUG \
             -frounding-math -fsignaling-nans -ffloat-store -ffp-contract=off"

# cmake .. -G"Ninja" \
#          -DFORCE_COLORED_OUTPUT=1 \
#          -DCMAKE_BUILD_TYPE=RELEASE \
#          -DCMAKE_C_COMPILER="clang" \
#          -DCMAKE_C_FLAGS="-fuse-ld=lld" \
#          -DCMAKE_CXX_COMPILER="clang++" \
#          -DCMAKE_CXX_FLAGS="\
#              -ftime-trace \
#              -fuse-ld=lld \
#              -Wall -Wextra -Wpedantic -Wno-braced-scalar-init \
#              -Wno-pragmas -Wno-missing-field-initializers \
#              -O3 -DNDEBUG \
#              -frounding-math -ffp-contract=off \
#              -g3 -ftime-trace" # TODO (P0): for profiling, remove
