#!/bin/bash
find ./src | grep pp | xargs clang-format -i
find ./include | grep pp | xargs clang-format -i
