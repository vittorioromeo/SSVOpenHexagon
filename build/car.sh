#!/bin/bash
ninja check && ninja && (cp ./SSVOpenHexagon.exe ../_RELEASE && cd ../_RELEASE && ./SSVOpenHexagon.exe)
