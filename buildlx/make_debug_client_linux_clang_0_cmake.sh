#!/bin/bash

set -e

echo ""
echo ""
echo "--------------------------------------------------------------------"
echo "| RUNNING CMAKE IN DEBUG MODE (NINJA)                              |"
echo "--------------------------------------------------------------------"
cmake .. -G"Ninja" \
         -DFORCE_COLORED_OUTPUT=1 \
         -DCMAKE_BUILD_TYPE=DEBUG \
         -DCMAKE_C_COMPILER="clang" \
         -DCMAKE_CXX_COMPILER="clang++" \
         -DCMAKE_CXX_FLAGS="\
            -O0 -fno-omit-frame-pointer \
            -Wall -Wextra -Wpedantic -Wno-braced-scalar-init -Wno-missing-field-initializers \
            -D_GLIBCXX_ASSERTIONS=1 \
            -fstack-protector -Wno-pragmas \
            -fsanitize=address"

