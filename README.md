##[Open Hexagon 2.0](http://www.facebook.com/OpenHexagon) - [by Vittorio Romeo](http://vittorioromeo.info) 

---

###[Official README](http://vittorioromeo.info/Downloads/OpenHexagon/README.html)  

---

## A C++11 compiler is required!

* `g++ 4.7.2` and `clang++ 3.2` should work properly.

* `g++ 4.8.1` and `clang++ 3.4` will give the best results.

---

## How to build on Linux (debian derivatives)

Tested on `Linux Mint 15 x64` and `Linux Mint 15 x86`.  

1. Clone this repository and initialize it
```bash
git clone git://github.com/SuperV1234/SSVOpenHexagon.git
cd SSVOpenHexagon
./init-repository.sh
```

2. If your distribution packages SFML 2 you can install it through your package manager otherwise build and install it [manually](http://sfmlcoder.wordpress.com/2011/08/16/building-sfml-2-0-with-make-for-gcc/) - you can also try the [Ubuntu PPA](https://github.com/SFML/ubuntu-sfml/wiki) or the [official binaries](http://sfml-dev.org/download/sfml/2.0/)

3. Install dependencies
```bash
sudo apt-get install liblua5.2-dev zlib1g-dev
```

4. Build dependencies and Open Hexagon
```bash
./build-repository-oh.sh
```

5. Download assets
```bash
./wget-assets.sh ./_RELEASE/
```

---

## How to build on Arch Linux

Tested on `Arch Linux x64` and `ArchBang x86`.

1. Clone this repository and initialize it
```bash
git clone git://github.com/SuperV1234/SSVOpenHexagon.git
cd SSVOpenHexagon
./init-repository.sh
```

2. Install dependencies 
```bash
sudo pacman -S sfml lua51 zlib
```

3. Build dependencies and Open Hexagon
```bash
cd SSVOpenHexagon
./build-repository-oh.sh
```

4. You may have to append `/usr/local/lib` to the `$PATH` or `$LD_LIBRARY_PATH` environment variables to allow Open Hexagon to find the required libraries - if that doesn't work, try:
```bash
sudo ldconfig /usr/local/lib  
```

5. Download assets
```bash
./wget-assets.sh ./_RELEASE/
```

---

## How to build on Windows

Tested on `Windows 8 x86`

1. Get [7-Zip](http://downloads.sourceforge.net/sevenzip/7z920.exe), [MinGW](http://sourceforge.net/projects/mingwbuilds/files/host-windows/releases/4.8.1/32-bit/threads-posix/dwarf/x32-4.8.1-release-posix-dwarf-rev2.7z/download) and add it to your `$PATH`, [CMake](http://www.cmake.org/files/v2.8/cmake-2.8.11.2-win32-x86.exe), [Git](http://git-scm.com/download/win), [UPX](http://upx.sourceforge.net/)

2. Clone this repository and open bash
```posh
git clone git://github.com/SuperV1234/SSVOpenHexagon.git
cd SSVOpenHexagon
```
3. Pull everything recursively using `git bash`
```bash
./init-repository.sh`
```

4. Get [SFML2.1](http://sfml-dev.org/), [Lua5.2 binaries](http://sourceforge.net/projects/luabinaries/files/), [Lua5.2 includes](http://sourceforge.net/projects/luabinaries/files/), [Zlib](http://www.zlib.net/) and extract in `SSVOpenHexagon/extlibs/`, manually adjusting paths to have the result shown below
```
extlibs/SSV*/
	...
extlibs/SFML/
extlibs/SFML/bin/
extlibs/SFML/lib/
extlibs/SFML/include/
	...
extlibs/lua/
extlibs/lua/include/
extlibs/lua/lib/
	...
extlibs/zlib/
extlibs/zlib/bin/
extlibs/zlib/include/
extlibs/zlib/lib/
```

5. Build with `./build-win.bat` and download assets manually from `http://vittorioromeo.info/Misc/Linked/OHResources/`