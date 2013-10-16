#!/bin/bash

# This bash script, called in a repository with submodules, builds and installs all submodules then the project itself
# RELEASE MODE, SKIPS RPATH
# Takes $1 destination directory parameter

if [ -z "${1}" ]; then
	echo "This script the installation path as an argument!"
	exit 1
fi

absolutePath=$(readlink -f ${1})

projectName="SSVOpenHexagon"
buildType="Release"
makeJobs="4"
destinationDir="${absolutePath}/"
cmakePrefixPath="${absolutePath}/temp"
cmakeFlags="-DCMAKE_BUILD_TYPE=${buildType} -DCMAKE_SKIP_BUILD_RPATH=True"

if [ ! -d "${destinationDir}" ]; then
  	echo "${destinationDir} does not exist!"
  	exit 1
fi

echo "Building a release (distribution) version of ${projectName}"
echo "cmakeFlags: ${cmakeFlags}"
echo "cmakePrefixPath: ${cmakePrefixPath}"
echo "destinationDir: ${destinationDir}"
echo "All files in ${destinationDir} will be removed"
echo "Building ${projectName} requires a C++11 compiler (like clang++-3.2 or g++-4.7.2)"

read -p "Continue? [Y/N] " -n 1
if [[ ! $REPLY =~ ^[Yy]$ ]]; then
    exit 1
fi

rm -Rf "${destinationDir}"*

dependencies=("SSVUtils" "SSVUtilsJson" "SSVMenuSystem" "SSVEntitySystem" "SSVLuaWrapper" "SSVStart") # List of extlibs to build in order

export DESTDIR="${cmakePrefixPath}"

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
	cmake ../ "${cmakeFlags}" || die 1 "cmake failed"

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
cmake ../ "${cmakeFlags}"
make "-j${makeJobs}"; make install "-j${makeJobs}"

cd ".."
echo "Finished building ${projectName}..."

mv "${cmakePrefixPath}/usr/local/games/SSVOpenHexagon/"* "${destinationDir}"

echo "Copying system libraries..."

x86Folder="${destinationDir}/x86/"
mkdir "${x86Folder}"

cp -av "${cmakePrefixPath}/usr/local/lib/"* "${x86Folder}"
echo "Removing temp directories..."
rm -Rf "${cmakePrefixPath}"

searchPaths=("/usr/local/lib/" "/usr/lib/" "/usr/lib/i386-linux-gnu/")

for s in "${searchPaths[@]}"; do
	cp -av "${s}"libsfml*[!d].so* "${x86Folder}"
	cp -av "${s}"libfreetype*.so* "${x86Folder}"
	cp -av "${s}"libjpeg*.so* "${x86Folder}"
	cp -av "${s}"liblua*.so* "${x86Folder}"
	cp -av "${s}"libGLEW*.so* "${x86Folder}"
	cp -av "${s}"libopenal*.so* "${x86Folder}"
	cp -av "${s}"libstdc++*.so* "${x86Folder}"
	cp -av "${s}"libsfml*.so* "${x86Folder}"
	cp -av "${s}"libsndfile*.so* "${x86Folder}"
	cp -av "${s}"libFLAC*.so* "${x86Folder}"
	cp -av "${s}"libogg.so* "${x86Folder}"
	cp -av "${s}"libvorbis*.so* "${x86Folder}"
	cp -av "${s}"ld-linux*.so* "${x86Folder}"
	cp -av "${s}"libc.so* "${x86Folder}"
	cp -av "${s}"libdrm.so* "${x86Folder}"
	cp -av "${s}"libgcc*.so* "${x86Folder}"
	cp -av "${s}"libglapi.so* "${x86Folder}"
	cp -av "${s}"libm.so* "${x86Folder}"
	cp -av "${s}"libpthread.so* "${x86Folder}"
	cp -av "${s}"librt.so* "${x86Folder}"
	cp -av "${s}"libz.so* "${x86Folder}"
	cp -av "${s}"libx*.so* "${x86Folder}"
	cp -av "${s}"libX*.so* "${x86Folder}"
	cp -av "${s}"libSM.so* "${x86Folder}"
	cp -av "${s}"libGL.so* "${x86Folder}"
	cp -av "${s}"libSSV.so* "${x86Folder}"
done

mv -f "${destinationDir}/SSVOpenHexagon" "${x86Folder}"
chmod +x "${x86Folder}/SSVOpenHexagon"

touch "${destinationDir}/OpenHexagon"
chmod +x "${destinationDir}/OpenHexagon"

echo '#!/bin/bash' > "${destinationDir}/OpenHexagon"
echo 'export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:./x86/; ./x86/SSVOpenHexagon' >> "${destinationDir}/OpenHexagon"

find "${x86Folder}" -name SSV*'.so' | xargs strip -s -g
find "${x86Folder}" -name 'SSVOpenHexagon'* | xargs strip -s -g

echo "Successfully finished building ${projectName}."