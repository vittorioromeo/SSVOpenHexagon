#!/bin/bash
mingw32-make check -j8 && mingw32-make -j8 && (cp ./SSVOpenHexagon.exe ../_RELEASE && cd ../_RELEASE && lldb -ex run ./SSVOpenHexagon.exe)

