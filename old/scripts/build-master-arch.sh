#!/bin/bash
pacman -Syyuu --noconfirm
pacman -S --noconfirm --needed base-devel git gcc cmake sfml lua zlib upx
pacman -Scc --noconfirm

git clone https://github.com/SuperV1234/SSVOpenHexagon
(cd SSVOpenHexagon; ./init-repository.sh; mkdir "/home/OH/"; ./build-repository-oh.sh; ./build-repository-rel.sh "/home/OH/")