#!/bin/sh

set -e

cd /root/obs-websocket

export GIT_HASH=$(git rev-parse --short HEAD)
export PKG_VERSION="$GIT_HASH-git"

if [ -n "${TRAVIS_TAG}" ]; then
	export PKG_VERSION="$TRAVIS_TAG"
fi

cd /root/obs-websocket/build

checkinstall -y --type=debian --fstrans=no --nodoc \
	--backup=no --deldoc=yes --install=no \
	--pkgname=obs-websocket --pkgversion="$PKG_VERSION" \
	--pkglicense="GPLv2.0" --maintainer="contact@slepin.fr" \
	--requires="obs-studio,libqt5websockets5" --pkggroup="video" \
	--pakdir="$(dirname $(pwd))/package"

ls -lh ./package
