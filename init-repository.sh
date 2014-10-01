#!/bin/bash

# This bash script, called in a repository with submodules, inits and pulls all submodules recursively

echo "Initializing all submodules..."
git submodule update --init --recursive

echo "Discarding all submodule changes..."
git submodule foreach --recursive "git fetch --all; git reset --hard origin/master"

echo "Recursively pulling all submodules..."
git submodule foreach --recursive git pull origin master --recurse-submodules