#!/bin/sh
set -ex

mkdir build && cd build

ls -lh ../../obs-studio/build/UI/obs-frontend-api

cmake .. \
  -DCMAKE_PREFIX_PATH=$(brew --prefix qt5)/lib/cmake \
  -DQt5Core_DIR=$(brew --prefix qt5)/lib/cmake/Qt5Core \
  -DQt5Widgets_DIR=$(brew --prefix qt5)/lib/cmake/Qt5Widgets \
  -DQt5WebSockets_DIR=$(brew --prefix qt5)/lib/cmake/Qt5WebSockets \
  -DLIBOBS_INCLUDE_DIR=../../obs-studio/libobs \
  -DOBS_FRONTEND_LIB="$(pwd)/../../obs-studio/build/UI/obs-frontend-api/libobs-frontend-api.dylib" \
  -DCMAKE_INSTALL_PREFIX=/usr \
&& make -j4

