#!/bin/bash

set -e

echo ""
echo ""
echo "--------------------------------------------------------------------"
echo "| COPYING ALL EXECUTABLES TO _RELEASE FOLDER                       |"
echo "--------------------------------------------------------------------"
echo ""

cp ./SSVOpenHexagon.exe ../_RELEASE
cp ./SSVOpenHexagon-Console.exe ../_RELEASE
cp ./OHWorkshopUploader.exe ../_RELEASE
cp ./OHServerControl.exe ../_RELEASE

echo ""
echo ""
echo "--------------------------------------------------------------------"
echo "| COPYING LIBRARIES                                                |"
echo "--------------------------------------------------------------------"
echo ""

./copylibs.sh
