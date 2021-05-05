#!/bin/bash

set -e

echo ""
echo ""
echo "--------------------------------------------------------------------"
echo "| COPYING TO WINDOWS DRIVE AS 'SSVOpenHexagonVbox'                 |"
echo "--------------------------------------------------------------------"
echo ""

cp ./SSVOpenHexagon /media/sf_C_DRIVE/OHWorkspace/SSVOpenHexagon/_RELEASE/SSVOpenHexagonVbox

echo ""
echo ""
echo "--------------------------------------------------------------------"
echo "| COPYING TO VBOX DRIVE AS 'SSVOpenHexagonVbox'                    |"
echo "--------------------------------------------------------------------"
echo ""

cp ./SSVOpenHexagon ../_RELEASE/SSVOpenHexagonVbox

