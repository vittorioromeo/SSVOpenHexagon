#!/bin/bash

# This bash script, called in a repository with submodules, builds and installs all submodules then the project itself

PROJECTNAME=${PWD##*/} # Project to build (current directory name)
BUILDTYPE="RELEASE" # Passed to CMake (CMAKE_BUILD_TYPE)
BUILDSHARED="TRUE" # Passed to CMake (LIBNAME_BUILD_SHARED_LIB)

LIBS=() # List of extlibs to build (gathered from ./extlibs/*)
for dir in ./extlibs/*; do LIBS+=(${dir##*/}); done	# Fill LIBS

# Builds a lib, with name $1 - calls CMake, make -j and make install -j
function buildLib 
{
	local LIBNAME="$1"
	local ULIBNAME=$(echo "$1" | tr '[:lower:]' '[:upper:]')

	echo "Building $ULIBNAME..." 
  	cd $LIBNAME # Enter lib main directory (where CMakeLists.txt is)
  	rm CMakeCache.txt # Remove CMakeCache.txt, in case an earlier (accidental) build was made in the main directory1
  	mkdir build; cd build # Create and move to the build directory
  	rm CMakeCache.txt # If the library was previously built, remove CMakeCache.txt

  	# Run CMake, make and make install
	cmake ../ -D"$ULIBNAME"_BUILD_SHARED_LIB=$BUILDSHARED -DCMAKE_BUILD_TYPE=$BUILDTYPE
	make -j; make install -j

	cd ../.. # Go back to extlibs directory
	echo "Finished building $ULIBNAME..."
}  

cd extlibs  # Start building... Enter extlibs, and build extlibs
for LIB in ${LIBS[*]}; do buildLib $LIB; done
cd .. # Now we are in the main folder
echo "Building $PROJECTNAME..." 
rm CMakeCache.txt # Remove CMakeCache.txt, in case an earlier (accidental) build was made in the main directory
mkdir build; cd build # Create and move to the build directory
rm CMakeCache.txt # If the library was previously built, remove CMakeCache.txt

## Run CMake, make and make install
cmake ../ -DCMAKE_BUILD_TYPE=$BUILDTYPE
make -j; make install -j

cd ..
echo "Finished building $PROJECTNAME..."