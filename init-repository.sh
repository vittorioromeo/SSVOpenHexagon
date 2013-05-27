#!/bin/bash

echo "Initializing all submodules..."
git submodule update --init --recursive

echo "Stashing all submodule changes..."
git submodule foreach --recursive git stash

echo "Recursively pulling all submodules..."
git submodule foreach --recursive git pull origin master --recurse-submodules