#!/bin/sh
set -ex

cd /root/obs-websocket

mkdir build && cd build
cmake -DCMAKE_INSTALL_PREFIX=/usr ..
make -j4
