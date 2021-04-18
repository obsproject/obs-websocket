#!/bin/sh
set -ex

echo "[obs-websocket] Running CMake.."
mkdir build && cd build
cmake -DCMAKE_INSTALL_PREFIX=/usr -DUSE_UBUNTU_FIX=true ..

echo "[obs-websocket] Building plugin.."
make -j4
