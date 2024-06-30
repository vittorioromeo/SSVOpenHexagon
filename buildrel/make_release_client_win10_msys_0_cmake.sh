#!/bin/bash

set -e

echo ""
echo ""
echo "--------------------------------------------------------------------"
echo "| RUNNING CMAKE IN RELEASE MODE (GCC + NINJA)                      |"
echo "--------------------------------------------------------------------"
echo ""

cmake .. -G"Ninja" \
         -DFORCE_COLORED_OUTPUT=1 \
         -DCMAKE_BUILD_TYPE=RelWithDebInfo \
         -DCMAKE_C_COMPILER="gcc" \
         -DCMAKE_C_FLAGS="-fuse-ld=lld" \
         -DCMAKE_CXX_COMPILER="g++" \
         -DCMAKE_CXX_FLAGS="\
             -fuse-ld=lld \
             -DSFML_ENABLE_LIFETIME_TRACKING=1 \
             -Wall -Wextra -Wpedantic -Wno-braced-scalar-init \
             -Wno-pragmas -Wno-missing-field-initializers -Wno-array-bounds -Wno-restrict \
             -Wno-stringop-overflow \
             -O3 -DNDEBUG -g3 \
             -frounding-math -fsignaling-nans -ffloat-store -ffp-contract=off \
             -ffold-simple-inlines -fimplicit-constexpr" \
         -DCMAKE_C_FLAGS="-Wno-attributes"
