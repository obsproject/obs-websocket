#!/bin/sh

OSTYPE=$(uname)

if [ "${OSTYPE}" != "Darwin" ]; then
    echo "[obs-websocket - Error] macOS build script can be run on Darwin-type OS only."
    exit 1
fi

HAS_CMAKE=$(type cmake 2>/dev/null)
HAS_GIT=$(type git 2>/dev/null)

if [ "${HAS_CMAKE}" == "" ]; then
    echo "[obs-websocket - Error] CMake not installed - please run 'install-dependencies-macos.sh' first."
    exit 1
fi

if [ "${HAS_GIT}" == "" ]; then
    echo "[obs-websocket - Error] Git not installed - please install Xcode developer tools or via Homebrew."
    exit 1
fi

# Build obs-studio
cd ..
echo "[obs-websocket] Cloning obs-studio from GitHub.."
git clone https://github.com/obsproject/obs-studio
cd obs-studio
OBSLatestTag=$(git describe --tags --abbrev=0)
git checkout $OBSLatestTag
mkdir build && cd build
echo "[obs-websocket] Building obs-studio.."
cmake .. \
	-DDISABLE_PLUGINS=true \
	-DCMAKE_PREFIX_PATH=/usr/local/opt/qt/lib/cmake \
&& make -j4
