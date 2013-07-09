##[Open Hexagon](http://www.facebook.com/OpenHexagon) - [by Vittorio Romeo](http://vittorioromeo.info) 

---

###[Official README](http://vittorioromeo.info/Downloads/OpenHexagon/README.html)  

---

## How to build on Linux (debian derivatives)

Tested on `Linux Mint 15 x64` and `Linux Mint 15 x86`.  
Tested compilers: **g++ 4.7.2**, **g++ 4.8.0**, **g++ 4.8.1**, **clang++ 3.2**, [**clang++ 3.4**](http://llvm.org/apt/).

1. Clone this repository
```bash
git clone git://github.com/SuperV1234/SSVOpenHexagon.git
cd SSVOpenHexagon
```

2. Pull everything recursively (submodules!)
```bash
./init-repository.sh`
```

4. If your distribution packages SFML 2 you can install it through your package manager otherwise build and install it [manually](http://sfmlcoder.wordpress.com/2011/08/16/building-sfml-2-0-with-make-for-gcc/) - you can also try the [Ubuntu PPA](https://github.com/SFML/ubuntu-sfml/wiki) or the [official binaries](http://sfml-dev.org/download/sfml/2.0/)

5. Open Hexagon requires `liblua5.1-dev` library to compile
```bash
sudo apt-get install liblua5.1-dev
```
6. Build dependencies and Open Hexagon
```bash
./build-repository-oh.sh
```

7. Download assets
```bash
./wget-assets.sh
```

8. Open Hexagon should now be installed on your system - play!
```bash
./_RELEASE/SSVOpenHexagon
```

---

## How to build on Arch Linux

1. Clone this repository
```bash
git clone git://github.com/SuperV1234/SSVOpenHexagon.git
cd SSVOpenHexagon
```

2. Pull everything recursively (submodules!)
```bash
./init-repository.sh`
```

4. Install dependencies 
```bash
sudo pacman -S sfml lua51
```

5. Build dependencies and Open Hexagon
```bash
cd SSVOpenHexagon
./build-repository-oh.sh
```

6. You may have to append `/usr/local/lib` to the `$PATH` or `$LD_LIBRARY_PATH` environment variables to allow Open Hexagon to find the required libraries - if that doesn't work, try:
```bash
sudo ldconfig /usr/local/lib  
```

7. Download assets
```bash
./wget-assets.sh ./_RELEASE/
```

8. Open Hexagon should now be installed on your system - play!
```bash
./_RELEASE/SSVOpenHexagon
```

---

## How to build on Windows

Tested on `Windows 8 x86`

1. Get [7-Zip](http://downloads.sourceforge.net/sevenzip/7z920.exe)

2. Get [MinGW](http://sourceforge.net/projects/mingwbuilds/files/host-windows/releases/4.8.1/32-bit/threads-posix/dwarf/x32-4.8.1-release-posix-dwarf-rev2.7z/download) and add it to your `$PATH`

3. Get [CMake](http://www.cmake.org/files/v2.8/cmake-2.8.11.2-win32-x86.exe)

4. Get [Git](http://git-scm.com/download/win)

5. Clone this repository and open bash
```posh
git clone git://github.com/SuperV1234/SSVOpenHexagon.git
cd SSVOpenHexagon
```

6. Pull everything recursively using `git bash`
```bash
./init-repository.sh`
```

7. Get [SFML](http://sfml-dev.org/download/sfml/2.0/SFML-2.0-windows-gcc-4.7-mingw-32bits.zip) and extract in `SSVOpenHexagon/extlibs/SFML/`

8. Get [Lua5.1 binaries](http://sourceforge.net/projects/luabinaries/files/5.1.4/Executables/lua5_1_4_Win32_bin.zip/download) and [Lua5.1 includes](http://sourceforge.net/projects/luabinaries/files/5.1.4/Executables/lua5_1_4_Win32_bin.zip/download) and extract them in `SSVOpenHexagon/extlibs/lua/`
```

9. Your `extlibs` directory should look like this, make the required changes
```
SSV*/ folders
	...

SFML/
	bin/
	lib/
	include/
	...

lua/
	include/
	lib/

```

5. Build dependencies and Open Hexagon using `cmake-gui`
```bash
cd SSVOpenHexagon
cmake-gui 
```

6. You may have to append `/usr/local/lib` to the `$PATH` or `$LD_LIBRARY_PATH` environment variables to allow Open Hexagon to find the required libraries - if that doesn't work, try:
```bash
sudo ldconfig /usr/local/lib  
```

7. Download assets
```bash
./wget-assets.sh ./_RELEASE/
```

8. Open Hexagon should now be installed on your system - play!
```bash
./_RELEASE/SSVOpenHexagon
```