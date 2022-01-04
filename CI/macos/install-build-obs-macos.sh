#!/bin/sh

OSTYPE=$(uname)

if [ "${OSTYPE}" != "Darwin" ]; then
	echo "[obs-websocket - Error] macOS obs-studio build script can be run on Darwin-type OS only."
	exit 1
fi

HAS_CMAKE=$(type cmake 2>/dev/null)
HAS_GIT=$(type git 2>/dev/null)

if [ "${HAS_CMAKE}" = "" ]; then
	echo "[obs-websocket - Error] CMake not installed - please run 'install-dependencies-macos.sh' first."
	exit 1
fi

if [ "${HAS_GIT}" = "" ]; then
	echo "[obs-websocket - Error] Git not installed - please install Xcode developer tools or via Homebrew."
	exit 1
fi

# Build obs-studio
cd ..
echo "[obs-websocket] Cloning obs-studio from GitHub.."
git clone https://github.com/obsproject/obs-studio
cd obs-studio
OBSLatestTag=$(git describe --tags --abbrev=0)
git checkout 27.1.3
mkdir build && cd build
echo "[obs-websocket] Building obs-studio.."
cmake .. \
	-DQTDIR=/tmp/obsdeps \
	-DDepsPath=/tmp/obsdeps \
	-DCMAKE_OSX_DEPLOYMENT_TARGET=10.13 \
	-DDISABLE_PLUGINS=true \
	-DENABLE_SCRIPTING=0 \
	-DCMAKE_PREFIX_PATH=/tmp/obsdeps/lib/cmake \
&& make -j4
