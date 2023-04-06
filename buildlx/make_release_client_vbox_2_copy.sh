#!/bin/bash

set -e

echo ""
echo ""
echo "--------------------------------------------------------------------"
echo "| COPYING TO WINDOWS DRIVE AS 'SSVOpenHexagonLinux'                 |"
echo "--------------------------------------------------------------------"
echo ""

cp ./SSVOpenHexagon /media/sf_C_DRIVE/OHWorkspace/SSVOpenHexagon/_RELEASE/SSVOpenHexagonLinux

echo ""
echo ""
echo "--------------------------------------------------------------------"
echo "| COPYING TO VBOX DRIVE AS 'SSVOpenHexagonLinux'                    |"
echo "--------------------------------------------------------------------"
echo ""

cp ./SSVOpenHexagon ../_RELEASE/SSVOpenHexagonLinux
cp ./OHServerControl ../_RELEASE/OHServerControlLinux
cp ./OHWorkshopUploader ../_RELEASE/OHWorkshopUploaderLinux

echo ""
echo ""
echo "--------------------------------------------------------------------"
echo "| COPYING DEPS TO VBOX DRIVE                                       |"
echo "--------------------------------------------------------------------"
echo ""

cp ./_deps/zlib-build/libz.so.1 ../_RELEASE
cp /lib/x86_64-linux-gnu/libopenal.so.1 ../_RELEASE
cp /lib/x86_64-linux-gnu/libFLAC.so.8 ../_RELEASE
