#!/bin/sh
set -ex

mkdir build && cd build
cmake .. -DCMAKE_PREFIX_PATH=$(brew --prefix qt5)/lib/cmake -DQt5WebSockets_DIR=$(brew --prefix qt5)/lib/cmake/Qt5WebSockets -DCMAKE_MODULE_PATH=$(brew --prefix qt5)/lib/cmake/ -DLIBOBS_INCLUDE_DIR=../obs-studio/libobs -DLIBOBS_LIB=../obs-studio/libobs -DCMAKE_INSTALL_PREFIX=/usr && make -j4

# /clang_64/lib/cmake/ ?
