# [Open Hexagon](http://www.facebook.com/OpenHexagon) - GitHub repository# 
##[by Vittorio Romeo](http://vittorioromeo.info) </br>
[Official README](http://vittorioromeo.info/Downloads/OpenHexagon/README.html)

----------

## How to build (Linux)

Tested on Linux Mint 15. </br>
Compiler used: **clang++ 3.4** ([DEB REPO](http://llvm.org/apt/))

1. Clone this repository </br>
`> git clone git://github.com/SuperV1234/SSVOpenHexagon.git` </br>
`> cd SSVOpenHexagon`

2. Pull everything recursively (submodules!) </br>
`> bash ./pullEverything.sh`

3. You should now have all submodules updated and pulled
4. Consider using `update-alternatives --config cc` and `update-alternatives --config cxx` to set **clang** and **clang++** as your default compilers, if GCC does not cooperate.

5. Build and install [**SFML**](http://sfmlcoder.wordpress.com/2011/08/16/building-sfml-2-0-with-make-for-gcc/ "**SFML**")

6. Build dependencies + Open Hexagon </br>
`> cd SSVOpenHexagon` </br>
`> bash ./buildAllLinux.sh`

7. Open Hexagon should now be installed on your system - download the assets from a released binary version [from my website](http://vittorioromeo.info) and put them in the installed game folder