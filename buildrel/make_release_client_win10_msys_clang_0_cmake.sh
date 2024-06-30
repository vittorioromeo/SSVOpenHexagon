#!/bin/bash

set -e

echo ""
echo ""
echo "--------------------------------------------------------------------"
echo "| RUNNING CMAKE IN RELEASE MODE (CLANG + NINJA)                    |"
echo "--------------------------------------------------------------------"
echo ""

cmake .. -G"Ninja" \
         -DFORCE_COLORED_OUTPUT=1 \
         -DCMAKE_BUILD_TYPE=RelWithDebInfo \
         -DCMAKE_C_COMPILER="clang" \
         -DCMAKE_C_FLAGS="-fuse-ld=lld" \
         -DCMAKE_CXX_COMPILER="clang++" \
         -DCMAKE_CXX_FLAGS="\
             -fuse-ld=lld \
             -ftime-trace \
             -DSFML_ENABLE_LIFETIME_TRACKING=1 \
             -Wall -Wextra -Wpedantic -Wno-braced-scalar-init \
             -Wno-pragmas -Wno-missing-field-initializers -Wno-array-bounds \
             -O3 -DNDEBUG -g3 \
             -Wno-unknown-warning-option \
             -Wno-deprecated-non-prototype -Wno-unknown-attributes -Wno-maybe-uninitialized \
             -Wno-unused-command-line-argument \
             -frounding-math -ffp-contract=off" \
         -DCMAKE_C_FLAGS="-Wno-deprecated-non-prototype -Wno-unknown-attributes"
