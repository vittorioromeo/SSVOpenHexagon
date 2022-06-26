#!/bin/bash

set -e

echo ""
echo ""
echo "--------------------------------------------------------------------"
echo "| COPYING LIBRARIES AND SOFT-LINKING PACKS                         |"
echo "--------------------------------------------------------------------"
echo ""

./copylibs.sh
(cd ./test && rm -Rf ./Packs/ && ln -sf ../../_RELEASE/Packs .)
echo "Done."

echo ""
echo ""
echo "--------------------------------------------------------------------"
echo "| BUILDING WITH NINJA                                              |"
echo "--------------------------------------------------------------------"
echo ""

ninja
ninja check
