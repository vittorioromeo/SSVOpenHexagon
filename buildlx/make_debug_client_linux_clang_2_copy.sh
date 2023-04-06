#!/bin/bash

set -e

echo ""
echo ""
echo "--------------------------------------------------------------------"
echo "| COPYING TO WINDOWS DRIVE AS 'SSVOpenHexagonLinuxDebug'           |"
echo "--------------------------------------------------------------------"
echo ""

cp ./SSVOpenHexagon /media/sf_C_DRIVE/OHWorkspace/SSVOpenHexagon/_RELEASE/SSVOpenHexagonLinuxDebug

echo ""
echo ""
echo "--------------------------------------------------------------------"
echo "| COPYING TO VBOX DRIVE AS 'SSVOpenHexagonLinuxDebug'              |"
echo "--------------------------------------------------------------------"
echo ""

cp ./SSVOpenHexagon ../_RELEASE/SSVOpenHexagonLinuxDebug
cp ./OHServerControl ../_RELEASE/OHServerControlLinuxDebug
cp ./OHWorkshopUploader ../_RELEASE/OHWorkshopUploaderLinuxDebug

echo ""
echo ""
echo "--------------------------------------------------------------------"
echo "| COPYING DEPS TO VBOX DRIVE                                       |"
echo "--------------------------------------------------------------------"
echo ""

cp ./_deps/zlib-build/libz.so.1 ../_RELEASE
