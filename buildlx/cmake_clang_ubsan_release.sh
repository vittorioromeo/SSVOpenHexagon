#!/bin/bash
cmake .. -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_BUILD_TYPE=RELEASE -DCMAKE_CXX_FLAGS="-Wno-deprecated-declarations -g -fsanitize=undefined -Wall -Wextra -Wpedantic -Wno-injected-class-name -Wno-gnu-zero-variadic-macro-arguments -Wno-braced-scalar-init -Wno-ambiguous-reversed-operator"


