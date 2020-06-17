# [Open Hexagon 2.0](http://www.facebook.com/OpenHexagon) - [by Vittorio Romeo](http://vittorioromeo.info)

## How to build on Windows

1. Get [`nuwen.net`'s MinGW Distro](https://nuwen.net/mingw.html) and install it.

2. Get [CMake](https://cmake.org/download/) and install it.

3. Run `open_distro_window.bat` from your MinGW installation to open a shell.

4. Clone this repository and initialize it:

    ```bash
    git clone git://github.com/SuperV1234/SSVOpenHexagon.git
    cd SSVOpenHexagon
    ./init-repository.sh
    ```

5. Create a build directory and `cd` into it:

    ```bash
    mkdir build
    cd build
    ```

6. Run CMake and build:

    ```bash
    cmake .. -G"MinGW Makefiles"
    make -j
    ```

7. Copy build artifacts to `_RELEASE` folder:

    ```bash
    cp ./_deps/sfml-build/lib/sfml-audio-2.dll ../_RELEASE
    cp ./_deps/sfml-build/lib/sfml-graphics-2.dll ../_RELEASE
    cp ./_deps/sfml-build/lib/sfml-network-2.dll ../_RELEASE
    cp ./_deps/sfml-build/lib/sfml-system-2.dll ../_RELEASE
    cp ./_deps/sfml-build/lib/sfml-window-2.dll ../_RELEASE
    cp ./_deps/sfml-src/extlibs/bin/x64/openal32.dll ../_RELEASE
    cp ./_deps/zlib-build/libzlib1.dll ../_RELEASE
    cp ./SSVOpenHexagon.exe ../_RELEASE
    ```

8. Run the game:

    ```bash
    cd ../_RELEASE
    ./SSVOpenHexagon.exe
    ```

9. (Optional) Download assets:

    ```bash
    # (from repository root)
    ./wget-assets.sh ./_RELEASE/
    ```

    *(or clone [SSVOpenHexagonAssets](https://github.com/SuperV1234/SSVOpenHexagonAssets))*

## How to build on Arch Linux

0. Install dependencies

    ```bash
    sudo pacman -S git make cmake gcc sfml
    ```
    
1. Clone this repository and initialize it:

    ```bash
    git clone git://github.com/SuperV1234/SSVOpenHexagon.git
    cd SSVOpenHexagon
    ./init-repository.sh
    ```

2. Create a build directory and `cd` into it:

    ```bash
    mkdir build
    cd build
    ```

3. Run CMake and build:

    ```bash
    cmake ..
    make -j
    ```
    
4. Install to `_RELEASE` folder and copy dependencies:

    ```bash
    sudo make install
    ```
    
5. Run the game:

    ```bash
    cd ../_RELEASE
    ./SSVOpenHexagon
    ```

6. (Optional) Download assets:

    ```bash
    # (from repository root)
    ./wget-assets.sh ./_RELEASE/
    ```

    *(or clone [SSVOpenHexagonAssets](https://github.com/SuperV1234/SSVOpenHexagonAssets))*
