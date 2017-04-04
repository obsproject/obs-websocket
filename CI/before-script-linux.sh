#!/bin/sh
set -ex

mkdir build && cd build
cat /usr/lib/x86_64-linux-gnu/cmake/Qt5WebSockets/Qt5WebSocketsConfig.cmake || :
cmake -DLIBOBS_INCLUDE_DIR="../../obs-studio/libobs" -DCMAKE_INSTALL_PREFIX=/usr ..
make -j4
