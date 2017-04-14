#!/bin/sh
set -ex

brew update

# OBS Studio deps
brew install ffmpeg
brew install libav

# qtwebsockets deps
brew install qt5

# Build obs-studio
cd ..
git clone --recursive https://github.com/jp9000/obs-studio
cd obs-studio
mkdir build && cd build
cmake .. \
  -DCMAKE_PREFIX_PATH=$(brew --prefix qt5)/lib/cmake \
&& make -j4

sudo make install
