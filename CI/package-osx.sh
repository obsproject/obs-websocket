#!/bin/sh

set -e

export GIT_HASH=$(git rev-parse --short HEAD)

export VERSION="$GIT_HASH-$TRAVIS_BRANCH"
if [ -n "${TRAVIS_TAG}" ]; then
	export VERSION="$TRAVIS_TAG"
fi

export FILENAME="obs-websocket-$VERSION-osx.pkg"

mkdir release

packagesbuild ./CI/osx/obs-websocket.pkgproj

mv ./release/obs-websocket.pkg ./release/$FILENAME
