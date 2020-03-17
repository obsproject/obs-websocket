#!/bin/sh

set -e

cd /root/obs-websocket

export GIT_HASH=$(git rev-parse --short HEAD)
export PKG_VERSION="1-$GIT_HASH-$TRAVIS_BRANCH-git"

if [ -n "${TRAVIS_TAG}" ]; then
	export PKG_VERSION="$TRAVIS_TAG"
fi

cd /root/obs-websocket/build

PAGER="cat" checkinstall -y --type=debian --fstrans=no --nodoc \
	--backup=no --deldoc=yes --install=no \
	--pkgname=obs-websocket --pkgversion="$PKG_VERSION" \
	--pkglicense="GPLv2.0" --maintainer="stephane.lepin@gmail.com" \
	--pkggroup="video" \
	--pkgsource="https://github.com/Palakis/obs-websocket" \
	--requires="obs-studio libqt5core5a libqt5widgets5 qt5-image-formats-plugins" \
	--pakdir="/package"

chmod ao+r /package/*
