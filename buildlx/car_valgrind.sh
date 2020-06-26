#!/bin/bash
make -j8 && (cp ./SSVOpenHexagon ../_RELEASE && cd ../_RELEASE && gdb ./SSVOpenHexagon)

