#!/bin/bash

echo "Recursively pulling all submodules..."
git submodule update --init --recursive
git submodule foreach git pull origin master --recurse-submodules

echo "Pulling last version..."
git pull origin master --recurse-submodules