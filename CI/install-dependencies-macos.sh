#!/bin/sh
set -ex

# OBS Studio deps
brew update
brew install ffmpeg
brew install libav

# qtwebsockets deps
# qt latest
brew install qt5

#echo "Qt path: $(find /usr/local/Cellar/qt5 -d 1 | tail -n 1)"


# Packages app
curl -L -O  http://s.sudre.free.fr/Software/files/Packages.dmg -f --retry 5 -C -
hdiutil attach ./Packages.dmg
sudo installer -pkg /Volumes/Packages\ 1.2.3/packages/Packages.pkg -target /
