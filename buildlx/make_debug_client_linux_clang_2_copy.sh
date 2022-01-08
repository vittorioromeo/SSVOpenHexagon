#!/bin/bash

set -e

echo ""
echo ""
echo "--------------------------------------------------------------------"
echo "| COPYING ALL EXECUTABLES TO _RELEASE FOLDER                       |"
echo "--------------------------------------------------------------------"
cp ./SSVOpenHexagon ../_RELEASE
cp ./OHWorkshopUploader ../_RELEASE
cp ./OHServerControl ../_RELEASE

echo ""
echo ""
echo "--------------------------------------------------------------------"
echo "| COPYING LIBRARIES                                                |"
echo "--------------------------------------------------------------------"
./copylibs.sh
