#!/bin/sh

set -e

echo "-- Preparing package build"
export QT_CELLAR_PREFIX="$(find /usr/local/Cellar/qt -d 1 | tail -n 1)"

export WS_LIB="/usr/local/opt/qt/lib/QtWebSockets.framework/QtWebSockets"
export NET_LIB="/usr/local/opt/qt/lib/QtNetwork.framework/QtNetwork"

export GIT_HASH=$(git rev-parse --short HEAD)

export VERSION="$GIT_HASH-$TRAVIS_BRANCH"
export LATEST_VERSION="$TRAVIS_BRANCH"
if [ -n "${TRAVIS_TAG}" ]; then
	export VERSION="$TRAVIS_TAG"
	export LATEST_VERSION="$TRAVIS_TAG"
fi

export FILENAME="obs-websocket-$VERSION.pkg"
export LATEST_FILENAME="obs-websocket-latest-$LATEST_VERSION.pkg"

echo "-- Copying Qt dependencies"
cp $WS_LIB ./build
cp $NET_LIB ./build

chmod +rw ./build/QtWebSockets ./build/QtNetwork

echo "-- Modifying QtNetwork"
install_name_tool \
	-change /usr/local/opt/qt/lib/QtNetwork.framework/Versions/5/QtNetwork @rpath/QtNetwork \
	-change $QT_CELLAR_PREFIX/lib/QtCore.framework/Versions/5/QtCore @rpath/QtCore \
	./build/QtNetwork

echo "-- Modifying QtWebSockets"
install_name_tool \
	-change /usr/local/opt/qt/lib/QtWebSockets.framework/Versions/5/QtWebSockets @rpath/QtWebSockets \
	-change $QT_CELLAR_PREFIX/lib/QtNetwork.framework/Versions/5/QtNetwork @rpath/QtNetwork \
	-change $QT_CELLAR_PREFIX/lib/QtCore.framework/Versions/5/QtCore @rpath/QtCore \
	./build/QtWebSockets

echo "-- Modifying obs-websocket.so"
install_name_tool \
	-change /usr/local/opt/qt/lib/QtWebSockets.framework/Versions/5/QtWebSockets @rpath/QtWebSockets \
	-change /usr/local/opt/qt/lib/QtWidgets.framework/Versions/5/QtWidgets @rpath/QtWidgets \
	-change /usr/local/opt/qt/lib/QtNetwork.framework/Versions/5/QtNetwork @rpath/QtNetwork \
	-change /usr/local/opt/qt/lib/QtGui.framework/Versions/5/QtGui @rpath/QtGui \
	-change /usr/local/opt/qt/lib/QtCore.framework/Versions/5/QtCore @rpath/QtCore \
	./build/obs-websocket.so

# Check if replacement worked
echo "-- Dependencies for QtNetwork"
otool -L ./build/QtNetwork
echo "-- Dependencies for QtWebSockets"
otool -L ./build/QtWebSockets
echo "-- Dependencies for obs-websocket"
otool -L ./build/obs-websocket.so

chmod -w ./build/QtWebSockets ./build/QtNetwork

echo "-- Actual package build"
packagesbuild ./CI/macos/obs-websocket.pkgproj

echo "-- Renaming obs-websocket.pkg to $FILENAME"
mv ./release/obs-websocket.pkg ./release/$FILENAME
cp ./release/$FILENAME ./release/$LATEST_FILENAME
