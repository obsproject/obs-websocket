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
BREW_PACKAGES=$(brew list)
BREW_DEPENDENCIES="ffmpeg libav cmake"

for DEPENDENCY in ${BREW_DEPENDENCIES}; do
    if echo "${BREW_PACKAGES}" | grep -q "^${DEPENDENCY}\$"; then
        echo "[obs-websocket] Upgrading OBS-Studio dependency '${DEPENDENCY}'.."
        brew upgrade ${DEPENDENCY} 2>/dev/null
    else
        echo "[obs-websocket] Installing OBS-Studio dependency '${DEPENDENCY}'.."
        brew install ${DEPENDENCY} 2>/dev/null
    fi
done

# qtwebsockets deps
echo "[obs-websocket] Installing obs-websocket dependency 'QT 5.10.1'.."
# =!= NOTICE =!=
# When building QT5 from sources on macOS 10.13+, use local qt5 formula:
# brew install ./CI/macos/qt.rb
# Pouring from the bottle is much quicker though, so use bottle for now.
# =!= NOTICE =!=

brew install https://raw.githubusercontent.com/Homebrew/homebrew-core/9a70413d137839de0054571e5f85fd07ee400955/Formula/qt.rb 2>/dev/null

# Pin this version of QT5 to avoid `brew upgrade`
# upgrading it to incompatible version
brew pin qt 2>/dev/null

# Fetch and install Packages app
# =!= NOTICE =!=
# Installs a LaunchDaemon under /Library/LaunchDaemons/fr.whitebox.packages.build.dispatcher.plist
# =!= NOTICE =!=

echo "[obs-websocket] Installing Packaging app (might require password due to 'sudo').."
curl -o './Packages.pkg' --retry-connrefused -s --retry-delay 1 'https://s3-us-west-2.amazonaws.com/obs-nightly/Packages.pkg'
sudo installer -pkg ./Packages.pkg -target /
