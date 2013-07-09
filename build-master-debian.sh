#!/bin/bash
apt-get -y install build-essential 
apt-get dist-upgrade
apt-get -y install g++ liblua5.1-dev git cmake libpthread-stubs0-dev libgl1-mesa-dev libx11-dev libxrandr-dev libfreetype6-dev libglew1.5-dev libjpeg8-dev libsndfile1-dev libopenal-dev

git clone https://github.com/LaurentGomila/SFML
(cd SFML; mkdir build; cd build; cmake ..; make -j4; make install -j4)

git clone https://github.com/SuperV1234/SSVOpenHexagon -b curveWalls
(cd SSVOpenHexagon; ./init-repository.sh; mkdir "/home/OH/"; ./build-repository-rel.sh "/home/OH/")