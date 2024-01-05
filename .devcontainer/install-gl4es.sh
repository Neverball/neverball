#!/bin/sh

[ "$GL4ES_COMMIT" ] || exit 1

git clone --no-tags --single-branch https://github.com/ptitSeb/gl4es.git "${GL4ES_DIR}" &&
cd "${GL4ES_DIR}" && {
    git checkout $GL4ES_COMMIT &&
    mkdir build &&
    cd build &&
    emcmake cmake .. -DCMAKE_BUILD_TYPE=RelWithDebInfo -DNOX11=ON -DNOEGL=ON -DSTATICLIB=ON &&
    make
}