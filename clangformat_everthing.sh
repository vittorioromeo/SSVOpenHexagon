#!/bin/bash
set -euo pipefail

files=$(find ./src ./include ./test  \
    -type f \( -name '*.hpp' -o -name '*.cpp' -o -name '*.inl' \) \
    ! -name 'json.hpp')

total=$(echo "$files" | wc -l)
echo "Formatting $total files..."

echo "$files" | xargs -P"$(nproc)" -I{} sh -c 'echo "  {}" && clang-format -i {}'

echo "Done."
