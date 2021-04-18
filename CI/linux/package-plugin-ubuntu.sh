#!/bin/bash

set -e

export GIT_HASH=$(git rev-parse --short HEAD)
export PKG_VERSION="1-$GIT_HASH-$BRANCH_SHORT_NAME-git"

if [[ $BRANCH_FULL_NAME =~ ^refs/tags/ ]]; then
	export PKG_VERSION="$BRANCH_SHORT_NAME"
	echo "[obs-websocket] Branch is a tag. Setting version to $PKG_VERSION."
fi

cd ./build

PAGER="cat" sudo checkinstall -y --type=debian --fstrans=no --nodoc \
	--backup=no --deldoc=yes --install=no \
	--pkgname=obs-websocket --pkgversion="$PKG_VERSION" \
	--pkglicense="GPLv2.0" --maintainer="stephane.lepin@gmail.com" \
	--pkggroup="video" \
	--pkgsource="https://github.com/Palakis/obs-websocket" \
	--requires="obs-studio \(\>= 26.1.0\), libqt5core5a, libqt5widgets5, qt5-image-formats-plugins" \
	--pakdir="../package"

sudo chmod ao+r ../package/*
