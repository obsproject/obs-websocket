#!/bin/sh
set -ex

brew update

# OBS Studio deps
brew install ffmpeg
brew install libav
brew install x264

# qtwebsockets deps
brew install qt5

# obs-websocket deps

# Build obs-studio
cd ..
git clone --recursive https://github.com/jp9000/obs-studio
cd obs-studio
mkdir build && cd build
cmake .. -DCMAKE_PREFIX_PATH=$(brew --prefix qt5) -DQt5WebSockets_DIR=$(brew --prefix qt5)/lib/cmake/Qt5WebSockets -DCMAKE_MODULE_PATH=$(brew --prefix qt5)/lib/cmake/ -DLIBOBS_INCLUDE_DIR=~/obs-studio/libobs -DLIBOBS_LIB=~/obs-studio/libobs && make
sudo make install

sudo ldconfig
