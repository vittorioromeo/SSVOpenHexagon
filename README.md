<a href="https://openhexagon.org" target="_blank">
    <p align="center">
        <img src="https://vittorioromeo.info/Misc/Linked/githubohlogo.png">
    </p>
</a>

> **Open Hexagon is a fast-paced and adrenaline-inducing paced arcade experience by [Vittorio Romeo](https://vittorioromeo.info). Designed for moddability and custom level creation.** Now available on [Steam](https://store.steampowered.com/app/1358090/)!

## Credits

### Source Contributors

- John Kline
- Zly | [@zly_u](https://twitter.com/zly_u)
- AlphaPromethium
- Misa "Info Teddy" Elizabeth | [@Info__Teddy](https://twitter.com/Info__Teddy)

### Testing

- [Maniac](https://www.youtube.com/channel/UCnEHReBWFQ_0_-Ro4TpH4Tw)

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

5. Execute the build script:

    ```bash
    sh ./build.sh
    ```

    List of arguments:
    - ```-r, --run```: Run the game after build completion.
    - ```-d, --debug```: Runs and debugs the game after build completion.
    - ```-g, --regenerate-cmake```: Regenerates CMake files in build folder to match current OS. This is automatic, but can be done manually if needed.
    - ```-jN```: Executes the `make` command using N threads. Default is 4.
    - ```--mingw-debug```: Sets `-DCMAKE_BUILD_TYPE=DEBUG` for the 'cmake' command."
    - ```--mingw-release```: Sets `-DCMAKE_BUILD_TYPE=RELEASE` for the 'cmake' command."
    - ```-h, --help```: Displays this help.

6. (Optional) Download assets:

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


## How to build on Debian Distributions
(Tested on Ubuntu 20.04 LTS and Linux Mint 20 Cinnamon)

0. Install dependencies

    ```bash
    sudo apt-get install git make cmake gcc g++
    ```

    On Linux, SFML relies on you to install [all the dependencies](https://www.sfml-dev.org/tutorials/2.5/compile-with-cmake.php) yourself, so execute the following command below to do so:

    ```bash
    sudo apt-get install libxrandr-dev libopengl-dev libudev-dev libfreetype-dev libopenal-dev libvorbis-dev libflac-dev
    ```

    Ensure you have the latest versions on all dependencies.

    **Tip:** If your distribution is not able to find ``libopengl-dev``, you can install two packages: ``libglm-dev`` and ``libglew-dev``. Install both of these and it should substitute for OpenGL.

1. Clone this repository with submodules:

    ```bash
    git clone --recurse-submodules git://github.com/SuperV1234/SSVOpenHexagon.git
    cd SSVOpenHexagon
    ```

2. Execute the build script:

    ```bash
    ./build.sh
    ```

    List of arguments:
    - ```-r, --run```: Run the game after build completion.
    - ```-d, --debug```: Runs and debugs the game after build completion.
    - ```-v, --valgrind```: Valgrinds the game after build completion.
    - ```-g, --regenerate-cmake```: Regenerates CMake files in build folder to match current OS. This is automatic, but can be done manually if needed.
    - ```-jN```: Executes the `make` command using N threads. Default is 4.
    - ```-h, --help```: Displays this help.


3. (Optional) Download assets:

    ```bash
    # (from repository root)
    ./wget-assets.sh ./_RELEASE/
    ```

    *(or clone [SSVOpenHexagonAssets](https://github.com/SuperV1234/SSVOpenHexagonAssets))*

