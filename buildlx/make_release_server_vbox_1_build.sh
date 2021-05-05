#!/bin/bash

set -e

echo ""
echo ""
echo "--------------------------------------------------------------------"
echo "| BUILDING WITH NINJA                                              |"
echo "--------------------------------------------------------------------"
echo ""

ninja
ninja check
