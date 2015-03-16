#!/bin/bash

WS_DIR=$(readlink -f ${1})
INIT="${2}"

function getAndBuildZlib
{
	(
		if [ -f "${WS_DIR}/zlib/zlib1.dll" ]; then
			echo "ZLib already present"			
		else
			cd "${WS_DIR}"
			mkdir ./zlib; cd ./zlib
			curl http://zlib.net/zlib128-dll.zip > ./zlib.zip
			unzip ./zlib.zip
		fi
	)
}

function getAndBuildLua
{
	(
		if [ -f "${WS_DIR}/lua/lua52.dll" ]; then
			echo "Lua binaries already present"			
		else			
			cd "${WS_DIR}"
			mkdir ./lua; cd ./lua
			curl http://joedf.users.sourceforge.net/luabuilds/lua-5.2.3_Win32_bin.zip > ./lua.zip
			unzip ./lua.zip
		fi

		if [ -f "${WS_DIR}/lua/src/lua.h" ]; then
			echo "Lua sources already present"			
		else
			cd "${WS_DIR}"
			mkdir ./lua; cd ./lua			
			curl http://www.lua.org/ftp/lua-5.2.4.tar.gz > ./lua.tar.gz
			tar -xvf ./lua.tar.gz
			cd ./lua-5.2.4/
			cp -R ./* ../
			cd ..
			cp -R ./src/ ./include/
		fi		
	)   
}

function getAndBuildSFML
{
	(
		cd "${WS_DIR}"
		git clone https://github.com/LaurentGomila/SFML
		cd ./SFML
		cmake . -G "MinGW Makefiles" \
			-DCMAKE_MAKE_PROGRAM="C:/MinGW/bin/mingw32-make.exe" \
			-DCMAKE_C_COMPILER="C:/MinGW/bin/gcc.exe" \
			-DCMAKE_CXX_COMPILER="C:/MinGW/bin/g++.exe"
		mingw32-make -j2 && mingw32-make install -j2
		mkdir "${WS_DIR}/SFML/bin"
		cp "C:/Program Files (x86)/SFML/bin/"sfml-*.dll "${WS_DIR}/SFML/bin"
	)   
}

function buildExtlib 
{
	( 
		echo "${1}"
		cd ./"${1}"
		mkdir ./build; cd ./build
		cmake .. -G "MinGW Makefiles" \
			-DCMAKE_MAKE_PROGRAM="C:/MinGW/bin/mingw32-make.exe" \
			-DCMAKE_C_COMPILER="C:/MinGW/bin/gcc.exe" \
			-DCMAKE_CXX_COMPILER="C:/MinGW/bin/g++.exe"
		mingw32-make -j2 && mingw32-make install -j2
	)
}

function buildAllExtlibs
{
	(
		cd ./extlibs
 
		for dir in ./*; do
			if [ -d "${dir}" ]; then 
				echo ""
				git reset HEAD --hard 
				git pull origin master
			fi
		done

		buildExtlib SSVUtils
		buildExtlib SSVUtilsJson
		buildExtlib SSVStart
		buildExtlib SSVMenuSystem
		buildExtlib SSVEntitySystem
		buildExtlib SSVLuaWrapper
	)
}

if [ "$INIT" = true ]; then
	getAndBuildZlib 
	getAndBuildLua
	getAndBuildSFML
fi

(
	SFML_DIR="${WS_DIR}/SFML"
	LUA_DIR="${WS_DIR}/lua"
	ZLIB_DIR="${WS_DIR}/zlib"

	cd "${WS_DIR}"
	
	if [ "$INIT" = true ]; then
		git clone https://github.com/SuperV1234/SSVOpenHexagon.git
	fi

	cd ./SSVOpenHexagon

	if [ "$INIT" = true ]; then
		./init-repository.sh
		buildAllExtlibs
	fi

	mkdir ./build; cd ./build && rm ./CMakeCache*
	cmake .. -G "MinGW Makefiles" \
		-DCMAKE_MAKE_PROGRAM="C:/MinGW/bin/mingw32-make.exe" \
		-DCMAKE_C_COMPILER="C:/MinGW/bin/gcc.exe" \
		-DCMAKE_CXX_COMPILER="C:/MinGW/bin/g++.exe" \
		-DSFML_INCLUDE_DIR="${SFML_DIR}/include" \
		-DSFML_AUDIO_LIBRARY="${SFML_DIR}/bin/sfml-audio-2.dll" \
		-DSFML_GRAPHICS_LIBRARY="${SFML_DIR}/bin/sfml-graphics-2.dll" \
		-DSFML_WINDOW_LIBRARY="${SFML_DIR}/bin/sfml-window-2.dll" \
		-DSFML_SYSTEM_LIBRARY="${SFML_DIR}/bin/sfml-system-2.dll" \
		-DSFML_NETWORK_LIBRARY="${SFML_DIR}/bin/sfml-network-2.dll" \
		-DLUA_LIBRARY="${LUA_DIR}/lua52.dll" \
		-DZLIB_LIBRARY="${ZLIB_DIR}/zlib1.dll" \
		-DCMAKE_CXX_FLAGS="-std=c++1y -w -fpermissive -O3 -DNDEBUG -lz -Wl,--stack,8194304" \
		-DCMAKE_CXX_FLAGS_RELEASE=""
 
	mingw32-make -j2 && mingw32-make install -j2
	cp ./SSVOpenHexagon.exe ../_RELEASE/
)

(
	cd ./_RELEASE/
	cp ../../SFML/bin/*.dll .
	cp ../../LUA/lib/*.dll .
	cp ../../zlib/lib/*.dll .
	strip ./*SSV*.dll -g -s
	rm ./scores.json
	rm ./log.txt
	rm ./users.json
	rm ./Profiles/*.json
)