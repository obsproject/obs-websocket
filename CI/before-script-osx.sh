#!/bin/sh
set -ex

mkdir build && cd build
ls -lh $(brew --prefix qt5)
cmake .. \
  -DCMAKE_PREFIX_PATH=$(brew --prefix qt5)/lib/cmake \
  -DQt5Core_DIR=$(brew --prefix qt5)/lib/cmake/Qt5Core \
  -DQt5Widgets_DIR=$(brew --prefix qt5)/lib/cmake/Qt5Widgets \
  -DQt5WebSockets_DIR=$(brew --prefix qt5)/lib/cmake/Qt5WebSockets \
  -DLIBOBS_INCLUDE_DIR=../../obs-studio/libobs \
  -DLIBOBS_LIB=../../obs-studio/build/libobs/libobs.dylib \
  -DCMAKE_INSTALL_PREFIX=/usr \
&& make -j4
