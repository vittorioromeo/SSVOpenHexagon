#!/bin/bash

# Project to build
PROJECTNAME="SSVOpenHexagon"
UPROJECTNAME=$(echo "$PROJECTNAME" | tr -s '[:lower:]' '[:upper:]')

# Passed to CMake (CMAKE_BUILD_TYPE)
BUILDTYPE="RELEASE" 

# Passed to CMake (LIBNAME_BUILD_SHARED_LIB)
BUILDSHARED="TRUE" 

# Passed to CMake (SPARSEHASH_INCLUDE_DIR) [you may have to set this manually]
SPARSEHASHINCLUDEDIR="/usr/include/google/"

# List of extlibs to build
LIBS=("SSVJsonCpp" "SSVUtils" "SSVUtilsJson" "SSVStart" "SSVEntitySystem" "SSVMenuSystem" "SSVLuaWrapper")

# Simple echo wrapper function
function prettyEcho
{
	echo "---------"
	echo "---------"
	echo "$1"
	echo "---------"
	echo "---------"
}

# Builds a lib, with name $1 - calls CMake, make -j and make install -j
function buildLib 
{
	local LIBNAME="$1"
	local ULIBNAME=$(echo "$1" | tr '[:lower:]' '[:upper:]')

	prettyEcho "Building $ULIBNAME..." 

	# Enter lib main directory (where CMakeLists.txt is)
  	cd $LIBNAME

  	# Remove CMakeCache.txt, in case an earlier (accidental) build was made in the main directory
  	rm CMakeCache.txt

  	# Create and move to the build directory
  	mkdir build
  	cd build

  	# If the library was previously built, remove CMakeCache.txt
  	rm CMakeCache.txt

  	## Run CMake, make and make install
	cmake ../ -D"$ULIBNAME"_BUILD_SHARED_LIB=$BUILDSHARED -DCMAKE_BUILD_TYPE=$BUILDTYPE -DSPARSEHASH_INCLUDE_DIR=$SPARSEHASHINCLUDEDIR
	make -j
	make install -j

	# Go back to extlibs directory
	cd ../..

	prettyEcho "Finished building $ULIBNAME..."
}  

# Start building...
# Enter extlibs, and build extlibs
cd extlibs 

for LIB in ${LIBS[*]} 
do 
	buildLib $LIB 
done

cd ..

# Now we are in the main folder
prettyEcho "Building $PROJECTNAME..." 

cmake CMakeLists.txt -DCMAKE_BUILD_TYPE=$BUILDTYPE -DSPARSEHASH_INCLUDE_DIR=$SPARSEHASHINCLUDEDIR
make -j
make install -j

prettyEcho "Finished building $PROJECTNAME..."