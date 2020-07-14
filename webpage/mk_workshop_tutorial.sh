#!/bin/bash

pandoc workshop_tutorial.md --css="github-pandoc.css" --toc -o workshop.html --standalone -V title:"" --metadata title="Open Hexagon Workshop Tutorial"
echo -e "<div><p align=\"center\"><img style=\"width: 100%; height: auto;\" src=\"https://vittorioromeo.info/Misc/Linked/ohworkshopheader.png\"></p></div>\n\n\n$(cat workshop.html)" > workshop.html
