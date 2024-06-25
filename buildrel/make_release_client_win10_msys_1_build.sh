#!/bin/bash

set -e

echo ""
echo ""
echo "--------------------------------------------------------------------"
echo "| BUILDING WITH NINJA                                              |"
echo "--------------------------------------------------------------------"
echo ""

ninja

echo ""
echo ""
echo "--------------------------------------------------------------------"
echo "| COPYING LIBRARIES AND SOFT-LINKING PACKS                         |"
echo "--------------------------------------------------------------------"
echo ""

./copylibs.sh
(cd ./test && rm -Rf ./Packs/ && ln -sf ../../_RELEASE/Packs .)
(cd ./test && rm -Rf ./Replays/ && ln -sf ../../_RELEASE/Replays .)
echo "Done."

echo ""
echo ""
echo "--------------------------------------------------------------------"
echo "| BUILDING TESTS WITH NINJA                                        |"
echo "--------------------------------------------------------------------"
echo ""

ninja check
