#!/bin/bash

set -e

./make_release_client_win10_msys_0_cmake.sh
./make_release_client_win10_msys_1_build.sh
./make_release_client_win10_msys_2_copy.sh

