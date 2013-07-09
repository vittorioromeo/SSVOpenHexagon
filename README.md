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

3. You should now have all submodules updated and pulled

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

7. Open Hexagon should now be installed on your system - download the assets from a released binary version [from my website](http://vittorioromeo.info) and put them in the installed game folder

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

3. You should now have all submodules updated and pulled

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

7. Open Hexagon should now be installed on your system - download the assets from a released binary version [from my website](http://vittorioromeo.info) and put them in the installed game folder