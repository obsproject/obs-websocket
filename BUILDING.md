# Compiling obs-websocket
## Prerequisites
You'll need [QT 5.9.0](https://download.qt.io/official_releases/qt/5.7/5.7.0/), CMake, and a working development environment for OBS Studio installed on your computer. 

## Windows
In cmake-gui, you'll have to set the following variables :
- **QTDIR** (path) : location of the Qt environment suited for your compiler and architecture
- **LIBOBS_INCLUDE_DIR** (path) : location of the libobs subfolder in the source code of OBS Studio
- **LIBOBS_LIB** (filepath) : location of the obs.lib file
- **OBS_FRONTEND_LIB** (filepath) : location of the obs-frontend-api.lib file

## Linux
On Debian/Ubuntu :  
```
sudo apt-get install libqt5websockets5-dev
git clone --recursive https://github.com/Palakis/obs-websocket.git
cd obs-websocket
mkdir build && cd build
cmake -DLIBOBS_INCLUDE_DIR="<path to the libobs sub-folder in obs-studio's source code>" -DCMAKE_INSTALL_PREFIX=/usr ..
make -j4
sudo make install
```

## OS X
Use of the Travis macOS CI scripts is recommended. Please note that these scripts install new software and can change several settings on your system. An existing obs-studio development environment is not required, as `install-dependencies-macos.sh` will install it for you.
Of course, you're encouraged to dig through the contents of these scripts to look for issues or specificities.
```
git clone --recursive https://github.com/Palakis/obs-websocket.git
cd obs-websocket
./CI/install-dependencies-macos.sh
./CI/build-macos.sh
./CI/package-macos.sh
```
This will result in a ready-to-use `obs-websocket.pkg` installer in the `release` subfolder.

## Automated Builds
- Windows : [![Automated Build status for Windows](https://ci.appveyor.com/api/projects/status/github/Palakis/obs-websocket)](https://ci.appveyor.com/project/Palakis/obs-websocket/history)
- Linux & OS X : [![Automated Build status for Linux & OS X](https://travis-ci.org/Palakis/obs-websocket.svg?branch=master)](https://travis-ci.org/Palakis/obs-websocket)
