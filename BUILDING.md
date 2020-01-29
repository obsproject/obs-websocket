# Compiling obs-websocket

## Prerequisites

You'll need [Qt 5.10.x](https://download.qt.io/official_releases/qt/5.10/),
[CMake](https://cmake.org/download/) and a working [OBS Studio development environment](https://obsproject.com/wiki/install-instructions) installed on your
computer.

## Windows

In cmake-gui, you'll have to set the following variables :

- **QTDIR** (path) : location of the Qt environment suited for your compiler and architecture
- **LIBOBS_INCLUDE_DIR** (path) : location of the libobs subfolder in the source code of OBS Studio
- **LIBOBS_LIB** (filepath) : location of the obs.lib file
- **OBS_FRONTEND_LIB** (filepath) : location of the obs-frontend-api.lib file

## Linux

On Debian/Ubuntu :

```shell
sudo apt-get install libboost-all-dev
git clone --recursive https://github.com/Palakis/obs-websocket.git
cd obs-websocket
mkdir build && cd build
cmake -DLIBOBS_INCLUDE_DIR="<path to the libobs sub-folder in obs-studio's source code>" -DCMAKE_INSTALL_PREFIX=/usr ..
make -j4
sudo make install
```

## OS X

As a prerequisite, you will need Xcode for your current OSX version, the Xcode command line tools, and [Homebrew](https://brew.sh/).
Homebrew's setup will guide you in getting your system set up, you should be good to go once Homebrew is successfully up and running.

Use of the Travis macOS CI scripts is recommended. Please note that these
scripts install new software and can change several settings on your system. An
existing obs-studio development environment is not required, as
`install-build-obs-macos.sh` will install it for you. If you already have a
working obs-studio development environment and have built obs-studio, you can
skip that script.

Of course, you're encouraged to dig through the contents of these scripts to
look for issues or specificities.

```shell
git clone --recursive https://github.com/Palakis/obs-websocket.git
cd obs-websocket
./CI/install-dependencies-macos.sh
./CI/install-build-obs-macos.sh
./CI/build-macos.sh
./CI/package-macos.sh
```

This will result in a ready-to-use `obs-websocket.pkg` installer in the `release` subfolder.

## Automated Builds

- Windows: [![Automated Build status for Windows](https://ci.appveyor.com/api/projects/status/github/Palakis/obs-websocket)](https://ci.appveyor.com/project/Palakis/obs-websocket/history)
- Linux: [![Automated Build status for Linux](https://travis-ci.org/Palakis/obs-websocket.svg?branch=master)](https://travis-ci.org/Palakis/obs-websocket)
- macOS: [![Automated Build status for macOS](https://img.shields.io/azure-devops/build/Palakis/obs-websocket/Palakis.obs-websocket.svg)](https://dev.azure.com/Palakis/obs-websocket/_build)
