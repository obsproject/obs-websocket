#!/bin/sh

set -e

echo "[obs-websocket] Preparing package build"
export QT_CELLAR_PREFIX="$(/usr/bin/find /usr/local/Cellar/qt -d 1 | sort -t '.' -k 1,1n -k 2,2n -k 3,3n | tail -n 1)"

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

echo "[obs-websocket] Copying Qt dependencies"
if [ ! -f ./build/$(basename $WS_LIB) ]; then cp $WS_LIB ./build; fi
if [ ! -f ./build/$(basename $NET_LIB) ]; then cp $NET_LIB ./build; fi

chmod +rw ./build/QtWebSockets ./build/QtNetwork

echo "[obs-websocket] Modifying QtNetwork"
install_name_tool \
	-id @rpath/QtNetwork \
	-change /usr/local/opt/qt/lib/QtNetwork.framework/Versions/5/QtNetwork @rpath/QtNetwork \
	-change $QT_CELLAR_PREFIX/lib/QtCore.framework/Versions/5/QtCore @rpath/QtCore \
	./build/QtNetwork

echo "[obs-websocket] Modifying QtWebSockets"
install_name_tool \
	-id @rpath/QtWebSockets \
	-change /usr/local/opt/qt/lib/QtWebSockets.framework/Versions/5/QtWebSockets @rpath/QtWebSockets \
	-change $QT_CELLAR_PREFIX/lib/QtNetwork.framework/Versions/5/QtNetwork @rpath/QtNetwork \
	-change $QT_CELLAR_PREFIX/lib/QtCore.framework/Versions/5/QtCore @rpath/QtCore \
	./build/QtWebSockets

echo "[obs-websocket] Modifying obs-websocket.so"
install_name_tool \
	-change /usr/local/opt/qt/lib/QtWebSockets.framework/Versions/5/QtWebSockets @rpath/QtWebSockets \
	-change /usr/local/opt/qt/lib/QtWidgets.framework/Versions/5/QtWidgets @rpath/QtWidgets \
	-change /usr/local/opt/qt/lib/QtNetwork.framework/Versions/5/QtNetwork @rpath/QtNetwork \
	-change /usr/local/opt/qt/lib/QtGui.framework/Versions/5/QtGui @rpath/QtGui \
	-change /usr/local/opt/qt/lib/QtCore.framework/Versions/5/QtCore @rpath/QtCore \
	./build/obs-websocket.so

# Check if replacement worked
echo "[obs-websocket] Dependencies for QtNetwork"
otool -L ./build/QtNetwork
echo "[obs-websocket] Dependencies for QtWebSockets"
otool -L ./build/QtWebSockets
echo "[obs-websocket] Dependencies for obs-websocket"
otool -L ./build/obs-websocket.so

chmod -w ./build/QtWebSockets ./build/QtNetwork

echo "[obs-websocket] Actual package build"
packagesbuild ./CI/macos/obs-websocket.pkgproj

echo "[obs-websocket] Renaming obs-websocket.pkg to $FILENAME"
mv ./release/obs-websocket.pkg ./release/$FILENAME
cp ./release/$FILENAME ./release/$LATEST_FILENAME
