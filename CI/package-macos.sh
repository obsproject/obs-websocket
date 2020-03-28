#!/bin/sh

set -e

OSTYPE=$(uname)

if [ "${OSTYPE}" != "Darwin" ]; then
    echo "[obs-websocket - Error] macOS package script can be run on Darwin-type OS only."
    exit 1
fi

echo "[obs-websocket] Preparing package build"
export QT_CELLAR_PREFIX="$(/usr/bin/find /usr/local/Cellar/qt -d 1 | sort -t '.' -k 1,1n -k 2,2n -k 3,3n | tail -n 1)"

export GIT_HASH=$(git rev-parse --short HEAD)
export GIT_BRANCH_OR_TAG=$(git name-rev --name-only HEAD | awk -F/ '{print $NF}')

export VERSION="$GIT_HASH-$GIT_BRANCH_OR_TAG"
export LATEST_VERSION="$GIT_BRANCH_OR_TAG"

export FILENAME="obs-websocket-$VERSION.pkg"

echo "[obs-websocket] Modifying obs-websocket.so"
install_name_tool \
	-add_rpath @executable_path/../Frameworks/QtWidgets.framework/Versions/5/ \
	-add_rpath @executable_path/../Frameworks/QtGui.framework/Versions/5/ \
	-add_rpath @executable_path/../Frameworks/QtCore.framework/Versions/5/ \
	-change /usr/local/opt/qt/lib/QtWidgets.framework/Versions/5/QtWidgets @rpath/QtWidgets \
	-change /usr/local/opt/qt/lib/QtGui.framework/Versions/5/QtGui @rpath/QtGui \
	-change /usr/local/opt/qt/lib/QtCore.framework/Versions/5/QtCore @rpath/QtCore \
	./build/obs-websocket.so

# Check if replacement worked
echo "[obs-websocket] Dependencies for obs-websocket"
otool -L ./build/obs-websocket.so

echo "[obs-websocket] Actual package build"
packagesbuild ./CI/macos/obs-websocket.pkgproj

echo "[obs-websocket] Renaming obs-websocket.pkg to $FILENAME"
mv ./release/obs-websocket.pkg ./release/$FILENAME
