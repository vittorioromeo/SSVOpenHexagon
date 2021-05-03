#!/bin/bash
cmake .. -G"MinGW Makefiles" -DCMAKE_BUILD_TYPE=RELEASE -DCMAKE_CXX_FLAGS="-g -gdwarf-2 -Wall -Wextra -Wpedantic" -DWIN_PROFILE=1 -DCMAKE_EXPORT_COMPILE_COMMANDS=1
