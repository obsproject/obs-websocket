#!/bin/sh

set -e

echo "-- Preparing package build"
export QT_PREFIX="$(find /usr/local/Cellar/qt5 -d 1 | tail -n 1)"

export WS_LIB="$QT_PREFIX/lib/QtWebSockets.framework/QtWebSockets"
export NET_LIB="$QT_PREFIX/lib/QtNetwork.framework/QtNetwork"

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
	-change /usr/local/opt/qt5/lib/QtNetwork.framework/Versions/5/QtNetwork @rpath/QtNetwork \
	-change "$QT_PREFIX/lib/QtCore.framework/Versions/5/QtCore" @rpath/QtCore \
	./build/QtNetwork

echo "-- Modifying QtWebSockets"
install_name_tool \
	-change /usr/local/opt/qt5/lib/QtWebSockets.framework/Versions/5/QtWebSockets @rpath/QtWebSockets \
	-change $QT_PREFIX/lib/QtNetwork.framework/Versions/5/QtNetwork @rpath/QtNetwork \
	-change $QT_PREFIX/lib/QtCore.framework/Versions/5/QtCore @rpath/QtCore \
	./build/QtWebSockets

echo "-- Modifying obs-websocket.so"
install_name_tool \
	-change /usr/local/opt/qt5/lib/QtWebSockets.framework/Versions/5/QtWebSockets @rpath/QtWebSockets \
	-change /usr/local/opt/qt5/lib/QtWidgets.framework/Versions/5/QtWidgets @rpath/QtWidgets \
	-change /usr/local/opt/qt5/lib/QtNetwork.framework/Versions/5/QtNetwork @rpath/QtNetwork \
	-change /usr/local/opt/qt5/lib/QtGui.framework/Versions/5/QtGui @rpath/QtGui \
	-change /usr/local/opt/qt5/lib/QtCore.framework/Versions/5/QtCore @rpath/QtCore \
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
packagesbuild ./CI/osx/obs-websocket.pkgproj

echo "-- Renaming obs-websocket.pkg to $FILENAME"
mv ./release/obs-websocket.pkg ./release/$FILENAME
cp ./release/$FILENAME ./release/$LATEST_FILENAME
