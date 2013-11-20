cd extlibs

cd SSVUtils
mkdir lib
cd lib
cmake .. -G "MinGW Makefiles"
mingw32-make -j4
mingw32-make install -j4
xcopy /y ..\lib\libSSVUtils.dll ..\..\..\_RELEASE\
cd ../..

cd SSVUtilsJson
mkdir lib
cd lib
cmake .. -G "MinGW Makefiles"
mingw32-make -j4
mingw32-make install -j4
xcopy /y ..\lib\libSSVUtilsJson.dll ..\..\..\_RELEASE\
cd ../..

cd SSVMenuSystem
mkdir lib
cd lib
cmake .. -G "MinGW Makefiles"
mingw32-make -j4
mingw32-make install -j4
xcopy /y ..\lib\libSSVMenuSystem.dll ..\..\..\_RELEASE\
cd ../..

cd SSVEntitySystem
mkdir lib
cd lib
cmake .. -G "MinGW Makefiles"
mingw32-make -j4
mingw32-make install -j4
xcopy /y ..\lib\libSSVEntitySystem.dll ..\..\..\_RELEASE\
cd ../..

cd SSVStart
mkdir lib
cd lib
cmake .. -G "MinGW Makefiles" 
mingw32-make -j4
mingw32-make install -j4
xcopy /y ..\lib\libSSVStart.dll ..\..\..\_RELEASE\
cd ../..

cd SSVLuaWrapper
mkdir lib
cd lib
cmake .. -G "MinGW Makefiles"
mingw32-make -j4
mingw32-make install -j4
xcopy /y ..\lib\libSSVLuaWrapper.dll ..\..\..\_RELEASE\
cd ../..

xcopy /s/y .\lua ..
xcopy /s/y .\SFML ..
xcopy /s/y .\zlib ..

xcopy /s/y .\lua ..\lua
xcopy /s/y .\SFML ..\SFML
xcopy /s/y .\zlib ..\zlib

xcopy /s/y .\lua ..\..\lua
xcopy /s/y .\SFML ..\..\SFML
xcopy /s/y .\zlib ..\..\zlib

cd ..

mkdir build
cd build
cmake .. -G "MinGW Makefiles"
mingw32-make -j4
mingw32-make install -j4

xcopy /s/y .\SSVOpenHexagon.exe ..\_RELEASE\
xcopy /s/y ..\lib\sfml-audio-2.dll ..\_RELEASE\
xcopy /s/y ..\lib\sfml-graphics-2.dll ..\_RELEASE\
xcopy /s/y ..\lib\sfml-network-2.dll ..\_RELEASE\
xcopy /s/y ..\lib\sfml-system-2.dll ..\_RELEASE\
xcopy /s/y ..\lib\sfml-window-2.dll ..\_RELEASE\
xcopy /s/y ..\lib\libsndfile-1.dll ..\_RELEASE\
xcopy /s/y ..\lib\lua5.1.dll ..\_RELEASE\
xcopy /s/y ..\lib\lua51.dll ..\_RELEASE\
xcopy /s/y ..\lib\openal32.dll ..\_RELEASE\
xcopy /s/y ..\lib\zlib.dll ..\_RELEASE\

strip ..\_RELEASE\*SSV*.dll -g -s
upx -9 ..\_RELEASE\*SSV*
