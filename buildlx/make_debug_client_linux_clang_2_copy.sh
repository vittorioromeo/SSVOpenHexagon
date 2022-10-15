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

cp ./_deps/sfml-build/lib/libsfml-audio-d.so.3.0 ../_RELEASE
cp ./_deps/sfml-build/lib/libsfml-network-d.so.3.0 ../_RELEASE
cp ./_deps/luajit-build/src/libluajit.so ../_RELEASE
cp ./_deps/zlib-build/libz.so.1 ../_RELEASE
cp ./_deps/libsodium-cmake-build/libsodium.so ../_RELEASE
cp ./_deps/imgui-sfml-build/libImGui-SFML.so ../_RELEASE
cp ./_deps/sfml-build/lib/libsfml-graphics-d.so.3.0 ../_RELEASE
cp ./_deps/sfml-build/lib/libsfml-window-d.so.3.0 ../_RELEASE
cp ./_deps/sfml-build/lib/libsfml-system-d.so.3.0 ../_RELEASE

echo ""
echo ""
echo "--------------------------------------------------------------------"
echo "| COPYING SYSTEM DEPS TO VBOX DRIVE                                |"
echo "--------------------------------------------------------------------"
echo ""

cp /usr/lib/libdl.so.2 ../_RELEASE
cp /usr/lib/libXcursor.so.1 ../_RELEASE
cp /usr/lib/libGL.so.1 ../_RELEASE
cp /usr/lib/libpthread.so.0 ../_RELEASE
cp /usr/lib/libstdc++.so.6 ../_RELEASE
cp /usr/lib/libm.so.6 ../_RELEASE
cp /usr/lib/libgcc_s.so.1 ../_RELEASE
cp /usr/lib/libc.so.6 ../_RELEASE
cp /usr/lib/libopenal.so.1 ../_RELEASE
cp /usr/lib/libvorbisenc.so.2 ../_RELEASE
cp /usr/lib/libvorbisfile.so.3 ../_RELEASE
cp /usr/lib/libvorbis.so.0 ../_RELEASE
cp /usr/lib/libogg.so.0 ../_RELEASE
cp /usr/lib/libFLAC.so.8 ../_RELEASE
cp /usr/lib64/ld-linux-x86-64.so.2 ../_RELEASE
cp /usr/lib/librt.so.1 ../_RELEASE
cp /usr/lib/libXrender.so.1 ../_RELEASE
cp /usr/lib/libXfixes.so.3 ../_RELEASE
cp /usr/lib/libX11.so.6 ../_RELEASE
cp /usr/lib/libfreetype.so.6 ../_RELEASE
cp /usr/lib/libXrandr.so.2 ../_RELEASE
cp /usr/lib/libudev.so.1 ../_RELEASE
cp /usr/lib/libGLdispatch.so.0 ../_RELEASE
cp /usr/lib/libGLX.so.0 ../_RELEASE
cp /usr/lib/libxcb.so.1 ../_RELEASE
cp /usr/lib/libbz2.so.1.0 ../_RELEASE
cp /usr/lib/libpng16.so.16 ../_RELEASE
cp /usr/lib/libharfbuzz.so.0 ../_RELEASE
cp /usr/lib/libbrotlidec.so.1 ../_RELEASE
cp /usr/lib/libXext.so.6 ../_RELEASE
cp /usr/lib/libXau.so.6 ../_RELEASE
cp /usr/lib/libXdmcp.so.6 ../_RELEASE
cp /usr/lib/libgraphite2.so.3 ../_RELEASE
cp /usr/lib/libglib-2.0.so.0 ../_RELEASE
cp /usr/lib/libbrotlicommon.so.1 ../_RELEASE
cp /usr/lib/libpcre.so.1 ../_RELEASE
