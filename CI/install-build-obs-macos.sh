#!/bin/sh
set -ex

# Build obs-studio
cd ..
git clone https://github.com/obsproject/obs-studio
cd obs-studio
OBSLatestTag=$(git describe --tags --abbrev=0)
git checkout $OBSLatestTag
mkdir build && cd build
cmake .. \
  -DDISABLE_PLUGINS=true \
  -DCMAKE_PREFIX_PATH=/usr/local/opt/qt/lib/cmake \
&& make -j4
