#!/bin/bash
pacman -Syyuu --noconfirm
pacman -S --noconfirm --needed base-devel git gcc cmake sfml lua51

git clone https://github.com/SuperV1234/SSVOpenHexagon -b curveWalls
(cd SSVOpenHexagon; ./init-repository.sh; mkdir "/home/OH/"; ./build-repository-oh.sh; ./build-repository-rel.sh "/home/OH/")