#!/bin/sh
set -ex

mkdir build && cd build

cmake .. \
  -DQTDIR=/usr/local/opt/qt5 \
  -DLIBOBS_INCLUDE_DIR=../../obs-studio/libobs \
  -DOBS_FRONTEND_LIB="$(pwd)/../../obs-studio/build/UI/obs-frontend-api/libobs-frontend-api.dylib" \
  -DCMAKE_INSTALL_PREFIX=/usr \
&& make -j4
