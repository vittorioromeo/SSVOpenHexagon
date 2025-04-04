#!/bin/bash
# ninja check && ninja && (cp ./SSVOpenHexagon.exe ../_RELEASE && cd ../_RELEASE && ./SSVOpenHexagon.exe)
ninja && (cp ./SSVOpenHexagon.exe ../_RELEASE && cd ../_RELEASE && gdb -ex run ./SSVOpenHexagon.exe)
