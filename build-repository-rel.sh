#!/bin/bash

# This bash script, called in a repository with submodules, builds and installs all submodules then the project itself
# RELEASE MODE, SKIPS RPATH
# Takes $1 destination directory parameter

PROJECTNAME=${PWD##*/} # Project to build (current directory name)
BUILDTYPE="RELEASE" # Passed to CMake (CMAKE_BUILD_TYPE)
BUILDSHARED="TRUE" # Passed to CMake (LIBNAME_BUILD_SHARED_LIB)
MAKEJOBS="4" # make -j...

CMAKEFLAGS="-DCMAKE_BUILD_TYPE=$BUILDTYPE -DCMAKE_SKIP_BUILD_RPATH=TRUE"

export DESTDIR="$1/"
export CMAKE_PREFIX_PATH="$1/usr/local/"

echo "building for release"
echo "CMAKEFLAGS = $CMAKEFLAGS"
echo "DESTDIR = $DESTDIR"
echo "CMAKE_PREFIX_PATH = $CMAKE_PREFIX_PATH"

read -p "Continue? " -n 1
if [[ ! $REPLY =~ ^[Yy]$ ]]; then
    exit 1
fi

rm -Rf "$1/"*

LIBS=("SSVJsonCpp" "SSVUtils" "SSVUtilsJson" "SSVMenuSystem" "SSVEntitySystem" "SSVLuaWrapper" "SSVStart") # List of extlibs to build in order

function warn() {
	echo "Error occured in: `pwd`"; echo "Error was: "$@
}

function die() {
	status=$1; shift; warn "$@" >&2; exit $status
}

# Builds a lib, with name $1 - calls CMake, make -j and make install -j
function buildLib
{
	local LIBNAME="$1"

	echo "Building $LIBNAME..."
  	cd $LIBNAME # Enter lib main directory (where CMakeLists.txt is)
  	mkdir build; cd build # Create and move to the build directory

	# Run CMake, make and make install
	cmake ../ -DBUILD_SHARED_LIB=$BUILDSHARED $CMAKEFLAGS || \
		die 1 "cmake failed"

	make -j$MAKEJOBS || \
		die 1 "make failed"

	make install -j$MAKEJOBS || \
		die 1 "make install failed"

	cd ../.. # Go back to extlibs directory
	echo "Finished building $LIBNAME..."
}

cd extlibs  # Start building... Enter extlibs, and build extlibs
for LIB in ${LIBS[*]}; do buildLib $LIB; done
cd .. # Now we are in the main folder
echo "Building $PROJECTNAME..."
mkdir build; cd build # Create and move to the build directory


## Run CMake, make and make install
cmake ../ $CMAKEFLAGS
make -j$MAKEJOBS; make install -j$MAKEJOBS

cd ..
echo "Finished building $PROJECTNAME..."
echo "copying libraries"

DESTLIBPATH="${CMAKE_PREFIX_PATH}games/SSVOpenHexagon/x86/"
mkdir "${DESTLIBPATH}"

echo $DESTLIBPATH
echo "${CMAKE_PREFIX_PATH}lib/*"

cp -av "${CMAKE_PREFIX_PATH}lib/"* "$DESTLIBPATH"
cp -av "/usr/local/lib/"libsfml*[!d].so* "$DESTLIBPATH"
cp -av "/usr/local/lib/"libfreetype*.so* "$DESTLIBPATH"
cp -av "/usr/local/lib/"libjpeg*.so* "$DESTLIBPATH"
cp -av "/usr/local/lib/"liblua*.so* "$DESTLIBPATH"
cp -av "/usr/local/lib/"libGLEW*.so* "$DESTLIBPATH"
cp -av "/usr/local/lib/"libopenal*.so* "$DESTLIBPATH"
cp -av "/usr/local/lib/"libstdc++*.so* "$DESTLIBPATH"
cp -av "/usr/local/lib/"libsfml*.so* "$DESTLIBPATH"
cp -av "/usr/lib/"libsfml*[!d].so* "$DESTLIBPATH"
cp -av "/usr/lib/"libfreetype*.so* "$DESTLIBPATH"
cp -av "/usr/lib/"libjpeg*.so* "$DESTLIBPATH"
cp -av "/usr/lib/"liblua*.so* "$DESTLIBPATH"
cp -av "/usr/lib/"libGLEW*.so* "$DESTLIBPATH"
cp -av "/usr/lib/"libopenal*.so* "$DESTLIBPATH"
cp -av "/usr/lib/"libstdc++*.so* "$DESTLIBPATH"
cp -av "/usr/lib/i386-linux-gnu/"libsfml*[!d].so* "$DESTLIBPATH"
cp -av "/usr/lib/i386-linux-gnu/"libfreetype*.so* "$DESTLIBPATH"
cp -av "/usr/lib/i386-linux-gnu/"libjpeg*.so* "$DESTLIBPATH"
cp -av "/usr/lib/i386-linux-gnu/"liblua*.so* "$DESTLIBPATH"
cp -av "/usr/lib/i386-linux-gnu/"libGLEW*.so* "$DESTLIBPATH"
cp -av "/usr/lib/i386-linux-gnu/"libopenal*.so* "$DESTLIBPATH"
cp -av "/usr/lib/i386-linux-gnu/"libstdc++*.so* "$DESTLIBPATH"

mv -f "${CMAKE_PREFIX_PATH}games/SSVOpenHexagon/SSVOpenHexagon" "${CMAKE_PREFIX_PATH}games/SSVOpenHexagon/x86"
chmod 777 "${CMAKE_PREFIX_PATH}games/SSVOpenHexagon/x86/SSVOpenHexagon"

touch "${CMAKE_PREFIX_PATH}games/SSVOpenHexagon/OpenHexagon"
chmod 777 "${CMAKE_PREFIX_PATH}games/SSVOpenHexagon/OpenHexagon"

echo '#!/bin/bash' > "${CMAKE_PREFIX_PATH}games/SSVOpenHexagon/OpenHexagon"
echo 'export LD_LIBRARY_PATH=./x86/; ./x86/SSVOpenHexagon' >> "${CMAKE_PREFIX_PATH}games/SSVOpenHexagon/OpenHexagon"

chmod a+x "${CMAKE_PREFIX_PATH}games/SSVOpenHexagon/OpenHexagon"
mv -f "${CMAKE_PREFIX_PATH}games/SSVOpenHexagon/OpenHexagon" "${CMAKE_PREFIX_PATH}games/SSVOpenHexagon/OpenHexagon.sh"

mv -f "${CMAKE_PREFIX_PATH}games/SSVOpenHexagon" "$1/"
rm "$1/"[!S]* -Rf

find "$1" -name *'.so' | xargs strip -s -g
find "$1" -name 'SSVOpenHexagon'* | xargs strip -s -g