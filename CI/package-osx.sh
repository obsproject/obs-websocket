#!/bin/sh

set -e

export WS_LIB="$(brew --prefix qt5)/lib/QtWebSockets.framework/QtWebSockets"

export GIT_HASH=$(git rev-parse --short HEAD)

export VERSION="$GIT_HASH-$TRAVIS_BRANCH"
if [ -n "${TRAVIS_TAG}" ]; then
	export VERSION="$TRAVIS_TAG"
fi

export FILENAME="obs-websocket-$VERSION-osx.pkg"

mkdir release
cp $WS_LIB ./release

packagesbuild ./CI/osx/obs-websocket.pkgproj

mv ./release/obs-websocket.pkg ./release/$FILENAME
