#!/bin/bash

set -e

./make_debug_client_linux_clang_0_cmake.sh
./make_debug_client_linux_clang_1_build.sh
./make_debug_client_linux_clang_2_copy.sh

