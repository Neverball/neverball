#!/bin/sh

if [ ! -d data/backup ]; then
    cd data &&
    mkdir backup &&
    mv \
        map-fwp \
        map-hard \
        map-medium \
        map-mym \
        map-mym2 \
        map-tones \
        set-fwp.txt \
        set-hard.txt \
        set-medium.txt \
        set-mym.txt \
        set-mym2.txt \
        set-tones.txt \
        shot-fwp \
        shot-hard \
        shot-medium \
        shot-mym \
        shot-mym2 \
        shot-tones \
        backup &&
    mkdir map-medium &&
    cp backup/map-medium/title.sol map-medium/title.sol
elif [ -d data/backup ]; then
    cd data &&
    rm -r map-medium &&
    mv backup/* . &&
    rmdir backup
fi
