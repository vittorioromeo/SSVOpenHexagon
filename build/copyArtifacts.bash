#!/bin/bash -u

# We need the user to specify an argument for where their MSYS folder is
if (( $# >= 1 )); then
	msysLoc=$1
else
	echo "Please enter the path to your installed MSYS folder:"
	read msysLoc
fi

if [[ $msysLoc == "" ]]; then
	echo "ERROR: No path is provided. Exiting" 1>&2
	exit 1
fi

if [[ ! -d ${msysLoc} ]]; then
	echo "ERROR: The path provided doesn't exist" 1>&2
	exit 1
fi

# Locate the _deps and RELEASE folders
dependenciesFolder="./_deps"
releaseFolder="../_RELEASE"

if [[ ! -d ${dependenciesFolder} ]]; then
	echo "ERROR: Cannot find _deps folder" 1>&2
	exit 1
fi
if [[ ! -d ${releaseFolder} ]]; then
	echo "ERROR: Cannot find _RELEASE folder" 1>&2
	exit 1
fi

echo "Copying build artifacts to ${releaseFolder}"
# Copy the _deps artifacts
cp ${dependenciesFolder}/imgui-sfml-build/libImGui-SFML.dll ${releaseFolder}
cp ${dependenciesFolder}/zlib-build/libzlib1.dll ${releaseFolder}
cp ${dependenciesFolder}/luajit-build/src/libluajit.dll ${releaseFolder}
cp ${dependenciesFolder}/sfml-build/lib/libsfml-audio-2.dll ${releaseFolder}
cp ${dependenciesFolder}/sfml-build/lib/libsfml-graphics-2.dll ${releaseFolder}
cp ${dependenciesFolder}/sfml-build/lib/libsfml-network-2.dll ${releaseFolder}
cp ${dependenciesFolder}/sfml-build/lib/libsfml-system-2.dll ${releaseFolder}
cp ${dependenciesFolder}/sfml-build/lib/libsfml-window-2.dll ${releaseFolder}
cp ${dependenciesFolder}/libsodium-cmake-build/libsodium.dll ${releaseFolder}
cp ${dependenciesFolder}/sfml-src/extlibs/bin/x64/openal32.dll ${releaseFolder}
# Copy some DLLs from our MSYS system
cp ${msysLoc}/mingw64/bin/libstdc++-6.dll ${releaseFolder}
cp ${msysLoc}/mingw64/bin/libgcc_s_seh-1.dll ${releaseFolder}
# Copy the executables over
cp ./OHWorkshopUploader.exe ${releaseFolder}
cp ./SSVOpenHexagon.exe ${releaseFolder}

echo "Done! Press Enter To Exit"
read pressEnterToExit
