cd extlibs

cd SSVUtils
mkdir lib
cd lib
cmake .. -G "MinGW Makefiles"
mingw32-make -j4
mingw32-make install -j4
cd ../..

cd SSVUtilsJson
mkdir lib
cd lib
cmake .. -G "MinGW Makefiles"
mingw32-make -j4
mingw32-make install -j4
cd ../..

cd SSVMenuSystem
mkdir lib
cd lib
cmake .. -G "MinGW Makefiles"
mingw32-make -j4
mingw32-make install -j4
cd ../..

cd SSVEntitySystem
mkdir lib
cd lib
cmake .. -G "MinGW Makefiles"
mingw32-make -j4
mingw32-make install -j4
cd ../..

cd SSVStart
mkdir lib
cd lib
cmake .. -G "MinGW Makefiles" 
mingw32-make -j4
mingw32-make install -j4
cd ../..

cd SSVLuaWrapper
mkdir lib
cd lib
cmake .. -G "MinGW Makefiles"
mingw32-make -j4
mingw32-make install -j4
cd ../..

cd ..

mkdir build
cd build
cmake .. -G "MinGW Makefiles"
mingw32-make -j4
mingw32-make install -j4

xcopy /s/y .\SSVOpenHexagon.exe ..\_RELEASE\
xcopy /s/y ..\..\SFML\lib\sfml-audio-2.dll ..\_RELEASE\
xcopy /s/y ..\..\SFML\lib\sfml-graphics-2.dll ..\_RELEASE\
xcopy /s/y ..\..\SFML\lib\sfml-network-2.dll ..\_RELEASE\
xcopy /s/y ..\..\SFML\lib\sfml-system-2.dll ..\_RELEASE\
xcopy /s/y ..\..\SFML\lib\sfml-window-2.dll ..\_RELEASE\
xcopy /s/y ..\..\SFML\lib\libsndfile-1.dll ..\_RELEASE\
xcopy /s/y ..\..\lua\lib\lua5.1.dll ..\_RELEASE\
xcopy /s/y ..\..\lua\lib\lua51.dll ..\_RELEASE\
xcopy /s/y ..\..\SFML\lib\openal32.dll ..\_RELEASE\
xcopy /s/y ..\..\zlib\lib\zlib.dll ..\_RELEASE\

strip ..\_RELEASE\*SSV*.dll -g -s
upx -9 ..\_RELEASE\*SSV*

rm ..\_RELEASE\scores.json
rm ..\_RELEASE\log.txt
rm ..\_RELEASE\users.json
rm ..\_RELEASE\Profiles\*.json
