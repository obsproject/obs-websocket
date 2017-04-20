# Compiling obs-websocket
## Prerequisites
You'll need [QT 5.7.0](https://download.qt.io/official_releases/qt/5.7/5.7.0/), CMake, and a working development environment for OBS Studio installed on your computer. 

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
*To do*

## Automated Builds
- Windows : [![Automated Build status for Windows](https://ci.appveyor.com/api/projects/status/github/Palakis/obs-websocket)](https://ci.appveyor.com/project/Palakis/obs-websocket/history)
- Linux & OS X : [![Automated Build status for Linux & OS X](https://travis-ci.org/Palakis/obs-websocket.svg?branch=master)](https://travis-ci.org/Palakis/obs-websocket)
