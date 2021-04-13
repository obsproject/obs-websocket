#!/bin/sh

OSTYPE=$(uname)

if [ "${OSTYPE}" != "Darwin" ]; then
    echo "[obs-websocket - Error] macOS install dependencies script can be run on Darwin-type OS only."
    exit 1
fi

HAS_BREW=$(type brew 2>/dev/null)

if [ "${HAS_BREW}" = "" ]; then
    echo "[obs-websocket - Error] Please install Homebrew (https://www.brew.sh/) to build obs-websocket on macOS."
    exit 1
fi

# OBS Studio deps
echo "[obs-websocket] Updating Homebrew.."
brew update >/dev/null
echo "[obs-websocket] Checking installed Homebrew formulas.."
BREW_PACKAGES=$(brew list --formula)
BREW_DEPENDENCIES="jack speexdsp ccache swig mbedtls"

for DEPENDENCY in ${BREW_DEPENDENCIES}; do
    if echo "${BREW_PACKAGES}" | grep -q "^${DEPENDENCY}\$"; then
        echo "[obs-websocket] Upgrading OBS-Studio dependency '${DEPENDENCY}'.."
        brew upgrade ${DEPENDENCY} 2>/dev/null
    else
        echo "[obs-websocket] Installing OBS-Studio dependency '${DEPENDENCY}'.."
        brew install ${DEPENDENCY} 2>/dev/null
    fi
done

# Fetch and install Packages app
# =!= NOTICE =!=
# Installs a LaunchDaemon under /Library/LaunchDaemons/fr.whitebox.packages.build.dispatcher.plist
# =!= NOTICE =!=

HAS_PACKAGES=$(type packagesbuild 2>/dev/null)

if [ "${HAS_PACKAGES}" = "" ]; then
    echo "[obs-websocket] Installing Packaging app (might require password due to 'sudo').."
    curl -L -O http://s.sudre.free.fr/Software/files/Packages.dmg
    sudo hdiutil attach ./Packages.dmg
    sudo installer -pkg /Volumes/Packages\ 1.2.9/Install\ Packages.pkg -target /
fi

# Qt deps
echo "[obs-websocket] Installing obs-websocket dependency 'Qt ${QT_VERSION}'.."
curl -L -O https://github.com/obsproject/obs-deps/releases/download/${OBS_DEPS_VERSION}/macos-qt-${QT_VERSION}-${OBS_DEPS_VERSION}.tar.gz
tar -xf ./macos-qt-${QT_VERSION}-${OBS_DEPS_VERSION}.tar.gz -C "/tmp"
xattr -r -d com.apple.quarantine /tmp/obsdeps

# OBS Deps
echo "[obs-websocket] Downloading and unpacking OBS dependencies"
wget --quiet --retry-connrefused --waitretry=1 https://github.com/obsproject/obs-deps/releases/download/${OBS_DEPS_VERSION}/macos-deps-${OBS_DEPS_VERSION}.tar.gz
tar -xf ./macos-deps-${OBS_DEPS_VERSION}.tar.gz -C /tmp