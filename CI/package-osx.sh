#!/bin/sh

set -e

export GIT_HASH=$(git rev-parse --short HEAD)
export FILENAME="obs-websocket-$GIT_HASH-$TRAVIS_BRANCH-osx.pkg"

mkdir release

packagesbuild ./CI/osx/obs-websocket.pkgproj

mv ./release/obs-websocket.pkg ./release/$FILENAME
