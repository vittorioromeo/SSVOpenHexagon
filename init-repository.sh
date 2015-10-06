#!/bin/bash

# This bash script, called in a repository with submodules, inits and pulls all submodules

echo "Initializing all submodules..."
git submodule update --init

echo "Discarding all submodule changes..."
git submodule foreach "git checkout . ; git reset --hard origin/master"

echo "Recursively pulling all submodules..."
git submodule foreach "git pull origin master"
