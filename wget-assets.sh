#!/bin/bash

# This bash script downloads the game assets

FOLDERNAME="Testing"

cd ./_RELEASE
wget -r -nH --cut-dirs=4 --no-parent -e robots=off --reject="index.html*" "http://vittorioromeo.info/Misc/Linked/OHResources/$FOLDERNAME"