#!/bin/sh

set -e

export WS_LIB="$(brew --prefix qt5)/lib/QtWebSockets.framework/QtWebSockets"
export NET_LIB="$(brew --prefix qt5)/lib/QtNetwork.framework/QtNetwork"

export GIT_HASH=$(git rev-parse --short HEAD)

export VERSION="$GIT_HASH-$TRAVIS_BRANCH"
if [ -n "${TRAVIS_TAG}" ]; then
	export VERSION="$TRAVIS_TAG"
fi

export FILENAME="obs-websocket-$VERSION-osx.pkg"

# TODO : put a loop in there
install_name_tool \
	-change "$(brew --prefix qt5)/lib/QtWebSockets.framework/Versions/5/QtWebSockets" @rpath/QtWebSockets \
	-change "$(brew --prefix qt5)/lib/QtWidgets.framework/Versions/5/QtWidgets" @rpath/QtWidgets \
	-change "$(brew --prefix qt5)/lib/QtNetwork.framework/Versions/5/QtNetwork" @rpath/QtNetwork \
	-change "$(brew --prefix qt5)/lib/QtGui.framework/Versions/5/QtGui" @rpath/QtGui \
	-change "$(brew --prefix qt5)/lib/QtCore.framework/Versions/5/QtCore" @rpath/QtCore \
	./build/obs-websocket.so

mkdir release
cp $WS_LIB ./release
cp $NET_LIB ./release

packagesbuild ./CI/osx/obs-websocket.pkgproj

mv ./release/obs-websocket.pkg ./release/$FILENAME
