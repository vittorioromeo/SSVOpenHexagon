#!/bin/bash
cmake .. -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_BUILD_TYPE=DEBUG -DCMAKE_CXX_FLAGS="-Wno-deprecated-declarations -g -fsanitize=memory -fno-omit-frame-pointer -Wall -Wextra -Wpedantic -Wno-injected-class-name -Wno-gnu-zero-variadic-macro-arguments -Wno-braced-scalar-init -Wno-ambiguous-reversed-operator"


