#!/bin/bash
pacman -Syyuu --noconfirm
pacman -S --noconfirm --needed base-devel git gcc cmake sfml lua51 zlib
pacman -Scc --noconfirm

git clone https://github.com/SuperV1234/SSVOpenHexagon
(cd SSVOpenHexagon; ./init-repository.sh; mkdir "/home/OH/"; ./build-repository-oh.sh; ./build-repository-rel.sh "/home/OH/")