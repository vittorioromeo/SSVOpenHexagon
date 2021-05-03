#!/bin/bash
TIME=`date "+%b-%d-%Y-%H-%M-%S"`                      
FILENAME=ohdb-$TIME.tar.gz                 
SRC=/home/vittorioromeo/OHWorkspace/SSVOpenHexagon/_RELEASE/ohdb.sqlite
DESDIR=/home/vittorioromeo/ohdb_backup

mkdir -p $DESDIR                             
tar -cpzf $DESDIR/$FILENAME $SRC
