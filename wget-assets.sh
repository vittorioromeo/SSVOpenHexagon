#!/bin/bash

# This bash script downloads the game assets
# Takes $1 destination directory parameter

FOLDERNAME="Testing"

export DESTDIR="$1/"

echo "downloading assets"
echo "DESTDIR = $DESTDIR"

read -p "Continue? " -n 1
if [[ ! $REPLY =~ ^[Yy]$ ]]; then
    exit 1
fi

cd "$1"
wget -r -nH --cut-dirs=4 --no-parent -e robots=off --reject="*index.html*" "http://vittorioromeo.info/Misc/Linked/OHResources/$FOLDERNAME/"