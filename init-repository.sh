#!/bin/bash

# This bash script, called in a repository with submodules, inits and pulls all submodules recursively

echo "Initializing all submodules..."
git submodule update --init --recursive

echo "Stashing all submodule changes..."
git submodule foreach --recursive git stash

echo "Recursively pulling all submodules..."
git submodule foreach --recursive git pull origin master --recurse-submodules