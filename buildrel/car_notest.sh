#!/bin/bash
make -j8 && (cp ./SSVOpenHexagon.exe ../_RELEASE && cd ../_RELEASE && ./SSVOpenHexagon.exe)

