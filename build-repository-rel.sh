#!/bin/bash

# This bash script, called in a repository with submodules, builds and installs all submodules then the project itself
# Takes $1 destination directory parameter
# Takes $2 superuser '-s' optional flag parameter
# Takes $3 release '-r' optional flag parameter (RELEASE MODE, SKIPS RPATH)

function askContinue() {
	read -p "Continue? [Y/N] " -n 1
	if [[ ! $REPLY =~ ^[Yy]$ ]]; then
	    exit 1
	else
		echo ""
	fi
}

if [[ "$#" == "0" ]]; then
  	echo "This script requires at least one argument."
  	echo "1) Installation path"
	echo "2) Optional flag '-s' - requires superuser permissions"
	exit 1
fi

absolutePath=$(readlink -f ${1})
echo "Installation path: ${absolutePath}"

flagSuperuser=false
flagRelease=false

if [[ "$2" == "-s" ]] || [[ "$3" == "-s" ]]; then
	flagSuperuser=true
fi

if [[ "$2" == "-r" ]] || [[ "$3" == "-r" ]]; then
	flagRelease=true
fi	

if [[ "$flagRelease" == true ]]; then
	echo "Release mode activated - RPATH will be skipped"
else
	echo "Non-release mode activated - RPATH will not be skipped"
fi

if [[ "$flagSuperuser" == true ]]; then
	echo "Superuser mode activated - SSV libraries will be installed"
else
	echo "Normal user mode activated - SSV libraries will be compiled in a temp folder"
fi

askContinue

projectName="SSVOpenHexagon"
buildType="Release"
makeJobs="4"
destinationDir="${absolutePath}/"
cmakePrefixPath="${absolutePath}/temp"
cmakeFlags="-DCMAKE_BUILD_TYPE=${buildType}"

if [[ "$flagRelease" == true ]]; then
	cmakeFlags="$cmakeFlags -DCMAKE_SKIP_BUILD_RPATH=True"
fi

echo "projectName: $projectName"
echo "buildType: $buildType"
echo "makeJobs: $makeJobs"
echo "destinationDir: $destinationDir"
echo "cmakePrefixPath: $cmakePrefixPath"
echo "cmakeFlags: $cmakeFlags"

askContinue

if [[ ! -d "${destinationDir}" ]]; then
  	echo "${destinationDir} does not exist!"
  	echo "Create ${destinationDir} with mkdir?"

	askContinue

	mkdir -p "${destinationDir}"
	if [[ $? -ne 0 ]] ; then
	    echo "Could not create $destinationDir"
	    exit 1
	fi
fi

echo "Building ${projectName} requires a C++11 compiler (like clang++-3.2 or g++-4.7.2)"
echo "All files in ${destinationDir} will be removed"

askContinue

rm -Rf "${destinationDir}"*

# List of extlibs to build in order
dependencies=("SSVUtils" "SSVMenuSystem" "SSVLuaWrapper" "SSVStart") 

export DESTDIR="${cmakePrefixPath}"

function warn() {
	echo "Error occured in: `pwd`"; echo "Error was: "$@
}

function die() {
	status=$1; shift; warn "$@" >&2; exit $status
}

dependenciesIncludeDirs=()

# Builds a lib, with name $1 - calls CMake, make -j and make install -j
function buildLib
{
	local libName="${1}"

	echo "Building ${libName}..."
  	cd "${libName}" # Enter lib main directory (where CMakeLists.txt is)
  	mkdir "build"; cd "build" # Create and move to the build directory

	# Run CMake, make and (optionally) sudo make install
	cmake ../ ${cmakeFlags} || die 1 "cmake failed"
	make "-j${makeJobs}" || die 1 "make failed"

	if [ "$flagSuperuser" == true ]; then
		sudo make install "-j${makeJobs}" || die 1 "make install failed"
	else	
		cd ../
		dependenciesIncludeDirs+=("$PWD")
		echo "Added $PWD to dependencies include directories"

		cd ./include
		dependenciesIncludeDirs+=("$PWD")
		echo "Added $PWD to dependencies include directories"		
	fi

	cd ../.. # Go back to extlibs directory
	echo "Finished building ${libName}..."
}

cd "extlibs"  # Start building... Enter extlibs, and build extlibs
for l in "${dependencies[@]}"; do buildLib "${l}"; done
cd ".." # Now we are in the main folder
echo "Building ${projectName}..."
mkdir "build"; cd "build" # Create and move to the build directory
echo "Setting additional CMake flags..."

if [[ "$flagSuperuser" == false ]]; then
	cmakeFlags="$cmakeFlags -DCMAKE_INCLUDE_PATH="
	for i in "${dependenciesIncludeDirs[@]}"; do
	   	cmakeFlags="${cmakeFlags}${i}:"
	done
fi

echo "final cmakeFlags: $cmakeFlags"

askContinue

## Run CMake, make and make install
cmake ../ ${cmakeFlags}
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
	# lddtree output for SSVOpenHexagon
	cp -av "${s}"libsfml-audio.so.* "${x86Folder}"
	cp -av "${s}"libopenal.so.* "${x86Folder}"
	cp -av "${s}"libdl.so.* "${x86Folder}"
	cp -av "${s}"libsndfile.so.* "${x86Folder}"
	cp -av "${s}"libFLAC.so.* "${x86Folder}"
	cp -av "${s}"libvorbisenc.so.* "${x86Folder}"
	cp -av "${s}"libvorbis.so.* "${x86Folder}"
	cp -av "${s}"libogg.so.* "${x86Folder}"
	cp -av "${s}"librt.so.* "${x86Folder}"
	cp -av "${s}"libsfml-graphics.so.* "${x86Folder}"
	cp -av "${s}"libfreetype.so.* "${x86Folder}"
	cp -av "${s}"libbz2.so.* "${x86Folder}"
	cp -av "${s}"libpng16.so.* "${x86Folder}"
	cp -av "${s}"libGLEW.so.* "${x86Folder}"
	cp -av "${s}"libXmu.so.* "${x86Folder}"
	cp -av "${s}"libXt.so.* "${x86Folder}"
	cp -av "${s}"libXi.so.* "${x86Folder}"
	cp -av "${s}"libjpeg.so.* "${x86Folder}"
	cp -av "${s}"libGL.so.* "${x86Folder}"
	cp -av "${s}"libnvidia-tls.so.* "${x86Folder}"
	cp -av "${s}"libnvidia-glcore.so.* "${x86Folder}"
	cp -av "${s}"libSM.so.* "${x86Folder}"
	cp -av "${s}"libuuid.so.* "${x86Folder}"
	cp -av "${s}"libICE.so.* "${x86Folder}"
	cp -av "${s}"libX11.so.* "${x86Folder}"
	cp -av "${s}"libxcb.so.* "${x86Folder}"
	cp -av "${s}"libXau.so.* "${x86Folder}"
	cp -av "${s}"libXdmcp.so.* "${x86Folder}"
	cp -av "${s}"libXext.so.* "${x86Folder}"
	cp -av "${s}"libXrandr.so.* "${x86Folder}"
	cp -av "${s}"libXrender.so.* "${x86Folder}"
	cp -av "${s}"libsfml-window.so.* "${x86Folder}"
	cp -av "${s}"libsfml-system.so.* "${x86Folder}"
	cp -av "${s}"libsfml-network.so.* "${x86Folder}"
	cp -av "${s}"liblua5.2.so.* "${x86Folder}"
	cp -av "${s}"liblua.so.* "${x86Folder}"
	cp -av "${s}"libz.so.* "${x86Folder}"
	cp -av "${s}"libstdc++.so.* "${x86Folder}"
	cp -av "${s}"libm.so.* "${x86Folder}"
	cp -av "${s}"libgcc_s.so.* "${x86Folder}"
	cp -av "${s}"libpthread.so.* "${x86Folder}"
	cp -av "${s}"libc.so.* "${x86Folder}"

	# cp -av "${s}"libsfml*[!d].so* "${x86Folder}"
	# cp -av "${s}"libfreetype*.so* "${x86Folder}"
	# cp -av "${s}"libjpeg*.so* "${x86Folder}"
	# cp -av "${s}"liblua*.so* "${x86Folder}"
	# cp -av "${s}"libGLEW*.so* "${x86Folder}"
	# cp -av "${s}"libopenal*.so* "${x86Folder}"
	# cp -av "${s}"libstdc++*.so* "${x86Folder}"
	# cp -av "${s}"libsfml*.so* "${x86Folder}"
	# cp -av "${s}"libsndfile*.so* "${x86Folder}"
	# cp -av "${s}"libFLAC*.so* "${x86Folder}"
	# cp -av "${s}"libogg.so* "${x86Folder}"
	# cp -av "${s}"libvorbis*.so* "${x86Folder}"
	# cp -av "${s}"ld-linux*.so* "${x86Folder}"
	# cp -av "${s}"libc.so* "${x86Folder}"
	# cp -av "${s}"libdrm.so* "${x86Folder}"
	# cp -av "${s}"libgcc*.so* "${x86Folder}"
	# cp -av "${s}"libglapi.so* "${x86Folder}"
	# cp -av "${s}"libm.so* "${x86Folder}"
	# cp -av "${s}"libpthread.so* "${x86Folder}"
	# cp -av "${s}"librt.so* "${x86Folder}"
	# cp -av "${s}"libz.so* "${x86Folder}"
	# cp -av "${s}"libx*.so* "${x86Folder}"
	# cp -av "${s}"libX*.so* "${x86Folder}"
	# cp -av "${s}"libpng*.so* "${x86Folder}"
	# cp -av "${s}"libSM.so* "${x86Folder}"
	# cp -av "${s}"libGL.so* "${x86Folder}"
	# cp -av "${s}"libSSV.so* "${x86Folder}" # shouldn't be needed anymore
done

mv -f "${destinationDir}/SSVOpenHexagon" "${x86Folder}"
chmod +x "${x86Folder}/SSVOpenHexagon"

touch "${destinationDir}/OpenHexagon"
chmod +x "${destinationDir}/OpenHexagon"

echo '#!/bin/bash' > "${destinationDir}/OpenHexagon"
echo 'export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:./x86/; ./x86/SSVOpenHexagon' >> "${destinationDir}/OpenHexagon"

find "${x86Folder}" -name SSV*'.so' | xargs strip -s -g
find "${x86Folder}" -name 'SSVOpenHexagon'* | xargs strip -s -g

find "${x86Folder}" -name SSV*'.so' | xargs upx -9
find "${x86Folder}" -name 'SSVOpenHexagon'* | xargs upx -9

rm "${destinationDir}/log.txt"
rm "${destinationDir}/scores.json"
rm "${destinationDir}/users.json"
rm "${destinationDir}/Profiles/*.json"

echo "Successfully finished building ${projectName}."
