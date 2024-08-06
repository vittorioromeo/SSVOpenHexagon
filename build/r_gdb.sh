#!/bin/bash
(cp ./SSVOpenHexagon.exe ../_RELEASE ; cd ../_RELEASE && gdb -- ./SSVOpenHexagon.exe "$@")
