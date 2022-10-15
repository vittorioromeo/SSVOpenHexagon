#!/bin/bash

set -e

./make_release_client_vbox_0_cmake.sh
./make_release_client_vbox_1_build.sh
./make_release_client_vbox_2_copy.sh

