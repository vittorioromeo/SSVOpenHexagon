#!/bin/bash
make check -j8 && make -j8 && (cp ./SSVOpenHexagon ../_RELEASE && cd ../_RELEASE && ./SSVOpenHexagon)

