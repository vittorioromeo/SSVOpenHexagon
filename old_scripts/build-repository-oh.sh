#!/bin/bash

# This bash script, called in a repository with submodules, builds and installs all submodules then the project itself
# Takes $1 destination directory parameter

projectName="SSVOpenHexagon"
buildType="Release"
makeJobs="4"
cmakeFlags="-DCMAKE_BUILD_TYPE=${buildType}"

echo "Building a release (non-distribution) version of ${projectName}"
echo "cmakeFlags: ${cmakeFlags}"
echo "Building ${projectName} requires a C++11 compiler (like clang++-3.2 or g++-4.7.2)"

read -p "Continue? [Y/N] " -n 1
if [[ ! $REPLY =~ ^[Yy]$ ]]; then
    exit 1
fi

dependencies=("SSVUtils" "SSVMenuSystem" "SSVEntitySystem" "SSVLuaWrapper" "SSVStart") # List of extlibs to build in order

function warn() {
	echo "Error occured in: `pwd`"; echo "Error was: "$@
}

function die() {
	status=$1; shift; warn "$@" >&2; exit $status
}

# Builds a lib, with name $1 - calls CMake, make -j and make install -j
function buildLib
{
	local libName="${1}"

	echo "Building ${libName}..."
  	cd "${libName}" # Enter lib main directory (where CMakeLists.txt is)
  	mkdir "build"; cd "build" # Create and move to the build directory

	# Run CMake, make and make install
	cmake ../ ${cmakeFlags} || die 1 "cmake failed"

	make "-j${makeJobs}" || die 1 "make failed"
	make install "-j${makeJobs}" || die 1 "make install failed"

	cd ../.. # Go back to extlibs directory
	echo "Finished building ${libName}..."
}

cd "extlibs"  # Start building... Enter extlibs, and build extlibs
for l in "${dependencies[@]}"; do buildLib "${l}"; done
cd ".." # Now we are in the main folder
echo "Building ${projectName}..."
mkdir "build"; cd "build" # Create and move to the build directory

## Run CMake, make and make install
cmake ../ ${cmakeFlags}
make "-j${makeJobs}"; make install "-j${makeJobs}"

echo "Successfully finished building ${projectName}."
