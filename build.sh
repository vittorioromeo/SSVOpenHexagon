#!/bin/bash
ROOT_DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )
BUILD_DIR="$ROOT_DIR/build"
RELEASE_DIR="$ROOT_DIR/_RELEASE"

# Argument variable declarations
REGENERATE_CMAKE=0
RUN=0
DEBUG=0
VALGRIND=0
MAKE_THREAD_COUNT="-j4"
DCMAKE_BUILD_TYPE=""

POSITIONAL=()

# Gets all arguments specified
while [[ $# -gt 0 ]]; do
    arg="$1"
    case $arg in
        -h|--help)
            echo "Usage:                     ./build.sh ..."
            echo "Arguments:"
            echo "    -r, --run              | Runs the game after build completion."
            echo "    -d, --debug            | Runs and debugs the game after build completion."
            echo "    -v, --valgrind         | Valgrinds the game after build completion. (Disabled for MinGW.)"
            echo "    -g, --regenerate-cmake | Regenerates CMake files in build folder to match current OS."
            echo "                              (This is automatic, but can be done manually if needed.)"
            echo "    -jN                    | Executes the 'make' command using N threads. Default is 4."
            echo "    --mingw-debug          | Sets -DCMAKE_BUILD_TYPE=DEBUG for the 'cmake' command. (Only for MinGW.)"
            echo "    --mingw-release        | Sets -DCMAKE_BUILD_TYPE=RELEASE for the 'cmake' command. (Only for MinGW.)"
            echo "    -h, --help             | Displays this help."
            echo ""
            exit 1
            ;;

        -r|--run)
            RUN=1
            shift
            ;;

        -d|--debug)
            DEBUG=1
            shift
            ;;

        -v|--valgrind)
            VALGRIND=1
            shift
            ;;
        
        -g|--regenerate-cmake)
            REGENERATE_CMAKE=1
            shift
            ;;

        -j*)
            MAKE_THREAD_COUNT=$arg
            shift
            ;;

        --mingw-debug)
            DCMAKE_BUILD_TYPE="-DCMAKE_BUILD_TYPE=DEBUG"
            shift
            ;;

        --mingw-release)
            DCMAKE_BUILD_TYPE="-DCMAKE_BUILD_TYPE=RELEASE"
            shift
            ;;

        *)
            POSITIONAL+=("$1")
            shift
            ;;
    esac
done

set -- "${POSITIONAL[@]}"

# Enters into the build directory
mkdir -p $BUILD_DIR
cd $BUILD_DIR

# Determines if the CMake files need to be regenerated to match the current OS
if [[ -f "cmake-ostype" ]]; then
    read -r CMAKE_OSTYPE < "cmake-ostype"
    if [[ $CMAKE_OSTYPE != $OSTYPE ]]; then
        REGENERATE_CMAKE=1
    fi
else
    REGENERATE_CMAKE=1
fi

case $OSTYPE in
    # MinGW / Windows...
    "msys")
        # Removes existing library artifacts
        rm -f "$RELEASE_DIR/sfml-audio-2.dll"
        rm -f "$RELEASE_DIR/sfml-graphics-2.dll"
        rm -f "$RELEASE_DIR/sfml-network-2.dll"
        rm -f "$RELEASE_DIR/sfml-system-2.dll"
        rm -f "$RELEASE_DIR/sfml-window-2.dll"
        rm -f "$RELEASE_DIR/openal32.dll"
        rm -f "$RELEASE_DIR/libzlib1.dll"
        rm -f "$RELEASE_DIR/OHWorkshopUploader.exe"
        rm -f "$RELEASE_DIR/SSVOpenHexagon.exe"

        # Disables the --valgrind argument.
        if [[ $VALGRIND -eq 1 ]]; then
            echo "Cannot valgrind in MinGW. Disabling this argument."
            VALGRIND=0
        fi

        if [[ $REGENERATE_CMAKE -eq 1 ]]; then
            echo "Regenerating CMake Files for MinGW/Windows..."
            find $BUILD_DIR -type f -not -wholename "$BUILD_DIR/*.sh" -print0 | xargs -0 rm --
            cmake .. -G "MinGW Makefiles" $DCMAKE_BUILD_TYPE
            echo $OSTYPE > "cmake-ostype"
        fi

        echo "Building for MinGW/Windows..."
        make $THREAD_MAKE_COUNT
        
        # Moves artifacts to release folder
        mv ./_deps/sfml-build/lib/sfml-audio-2.dll $RELEASE_DIR
        mv ./_deps/sfml-build/lib/sfml-graphics-2.dll $RELEASE_DIR
        mv ./_deps/sfml-build/lib/sfml-network-2.dll $RELEASE_DIR
        mv ./_deps/sfml-build/lib/sfml-system-2.dll $RELEASE_DIR
        mv ./_deps/sfml-build/lib/sfml-window-2.dll $RELEASE_DIR
        cp ./_deps/sfml-src/extlibs/bin/x64/openal32.dll $RELEASE_DIR
        mv ./_deps/zlib-build/libzlib1.dll $RELEASE_DIR
        mv ./OHWorkshopUploader.exe $RELEASE_DIR
        mv ./SSVOpenHexagon.exe $RELEASE_DIR
        ;;
    
    # Linux...
    "linux-gnu")
        # Removes existing executables
        rm -f "$RELEASE_DIR/OHWorkshopUploader"
        rm -f "$RELEASE_DIR/SSVOpenHexagon"

        # Disables the --mingw-debug and --mingw-release arguments.
        if [[ $DCMAKE_BUILD_TYPE != "" ]]; then
            echo "--mingw-debug and --mingw-release are only for MinGW. Disabling these arguments."
            DCMAKE_BUILD_TYPE=""
        fi

        if [[ $REGENERATE_CMAKE -eq 1 ]]; then
            echo "Regenerating CMake Files for Linux..."
            find $BUILD_DIR -type f -not -wholename "$BUILD_DIR/*.sh" -print0 | xargs -0 rm --
            cmake ..
            echo $OSTYPE > "cmake-ostype"
        fi

        echo "Building for Linux..."
        make $MAKE_THREAD_COUNT

        mv SSVOpenHexagon $RELEASE_DIR
        mv OHWorkshopUploader $RELEASE_DIR
        ;;

    *)
        echo "============================================================================"
        echo "Build script unsupported for '$OSTYPE' systems."
        echo "Feel free to yell at us and we will get this working for your system soon(tm)!"
        echo "(...don't actually yell.)"
        exit 2;
esac

# Move to release folder and run the game if it has the following arguments...
cd $RELEASE_DIR

if [[ $DEBUG -eq 1 ]]; then
    gdb ./SSVOpenHexagon.exe
elif [[ $VALGRIND -eq 1 ]]; then
    valgrind ./SSVOpenHexagon.exe
elif [[ $RUN -eq 1 ]]; then
    ./SSVOpenHexagon.exe
fi
