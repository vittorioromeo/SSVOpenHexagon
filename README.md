# [Open Hexagon](http://www.facebook.com/OpenHexagon) -  [by Vittorio Romeo](http://vittorioromeo.info) <br/>
[Official README](http://vittorioromeo.info/Downloads/OpenHexagon/README.html)

<br/>

## How to build (Linux)

Tested on `Linux Mint 15 x64` and `Linux Mint 15 x86`. <br/>
Tested compilers: **g++ 4.7.2**, **g++ 4.8.0**, **g++ 4.8.1**, **clang++ 3.2**, [**clang++ 3.4**](http://llvm.org/apt/)

1. Clone this repository <br/>
`> git clone git://github.com/SuperV1234/SSVOpenHexagon.git` <br/>
`> cd SSVOpenHexagon`

2. Pull everything recursively (submodules!) <br/>
`> bash ./init-repository.sh`

3. You should now have all submodules updated and pulled

4. Build and install [**SFML**](http://sfmlcoder.wordpress.com/2011/08/16/building-sfml-2-0-with-make-for-gcc/)

5. Open Hexagon requires `liblua5.1-dev` and `sparsehash` libraries to compile <br/>
`> apt-get install liblua5.1-dev` <br/>
Get latest sparsehash package [from here](https://code.google.com/p/sparsehash/downloads/list)

6. Build dependencies + Open Hexagon <br/>
`> cd SSVOpenHexagon` <br/>
`> bash ./build-repository.sh`

8. Open Hexagon should now be installed on your system - download the assets from a released binary version [from my website](http://vittorioromeo.info) and put them in the installed game folder