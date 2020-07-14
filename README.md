<a href="https://openhexagon.org" target="_blank">
    <p align="center">
        <img src="https://vittorioromeo.info/Misc/Linked/githubohlogo.png">
    </p>
</a>

> **Open Hexagon is a fast-paced and adrenaline-inducing paced arcade experience by [Vittorio Romeo](https://vittorioromeo.info). Designed for moddability and custom level creation.** Now available on [Steam](https://store.steampowered.com/app/1358090/)!

## How to build on Windows

1. Get [`nuwen.net`'s MinGW Distro](https://nuwen.net/mingw.html) and install it.

2. Get [CMake](https://cmake.org/download/) and install it.

3. Run `open_distro_window.bat` from your MinGW installation to open a shell. (Alternatively, you can do this in [Git Bash](https://gitforwindows.org/) if you have all the necessary libraries)

    * If you are using the command prompt, make sure that you are running bash by typing in `bash.exe`

4. Clone this repository with submodules:

    ```bash
    git clone --recurse-submodules --remote-submodules git://github.com/SuperV1234/SSVOpenHexagon.git
    cd SSVOpenHexagon
    ```
    
    **Note:** Not all `git` versions can recognize `--remote-submodules`. If you are cloning this git simply just to compile, you can omit this and you'll be fine. Most IDEs and the MinGW library offered in step 1 should be able to recognize `--remote-submodules`

5. `cd` into the build folder:

    ```bash
    cd SSVOpenHexagon/build
    ```

6. Run CMake and build:

    ```bash
    cmake .. -G"MinGW Makefiles"
    make -j8
    ```
    
    **Note 1:** If your bash is not recognizing CMake, check your environment variables to make sure that CMake is in there. If you still can't get it to work with bash, you can use ``cmd.exe`` and it should work just fine

    **Note 2:** Only use ``-j8`` if you are confident your PC is powerful enough to compile the instructions on multiple threads. If you have a lower end PC, lower the number of threads or remove the flag entirely. This will result in longer compiling times but will ensure you aren't overworking your PC.

7. Copy build artifacts to `_RELEASE` folder (You will have to use bash to do this):

    ```bash
    cp ./_deps/sfml-build/lib/sfml-audio-2.dll ../_RELEASE
    cp ./_deps/sfml-build/lib/sfml-graphics-2.dll ../_RELEASE
    cp ./_deps/sfml-build/lib/sfml-network-2.dll ../_RELEASE
    cp ./_deps/sfml-build/lib/sfml-system-2.dll ../_RELEASE
    cp ./_deps/sfml-build/lib/sfml-window-2.dll ../_RELEASE
    cp ./_deps/sfml-src/extlibs/bin/x64/openal32.dll ../_RELEASE
    cp ./_deps/zlib-build/libzlib1.dll ../_RELEASE
    cp ./OHWorkshopUploader.exe ../_RELEASE
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
    
    **Note:** If this file is producing errors, make sure the file is using UNIX line endings and not Windows Line Endings. Bash recognizes files primarily with UNIX Line Endings.

## How to build on Arch Linux

0. Install dependencies

    ```bash
    sudo pacman -S git make cmake gcc sfml
    ```
    
1. Clone this repository with submodules:

    ```bash
    git clone --recurse-submodules --remote-submodules git://github.com/SuperV1234/SSVOpenHexagon.git
    cd SSVOpenHexagon
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
