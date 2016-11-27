obs-websocket
==============
Websocket API for OBS Studio.

## How to use
The Websocket API server runs on port 4444 and a settings window is available in "Websocket server settings" under OBS' "Tools" menu. The obs-websocket protocol is documented in [PROTOCOL.md](PROTOCOL.md).  

Here's a list of available language APIs for obs-websocket :
- Javascript (browser & nodejs) : [obs-websocket-js](https://github.com/haganbmj/obs-websocket-js) by haganbmj

There's currently no frontend available for obs-websocket.

## Possible use cases
- Remote control OBS from a phone or tablet on the same local network
- Change your stream overlay/graphics based on the current scene (like the AGDQ overlay does)
- Automate scene switching with a third-party program (e.g. : auto-pilot, foot pedal, ...)

## Build instructions
You'll need QT 5 with QtWebSockets, CMake, and a working development environment for OBS Studio installed on your computer. 

### Windows
In cmake-gui, you'll have to set the following variables :
- **QTDIR** (path) : location of the Qt environment suited for your compiler and architecture
- **LIBOBS_INCLUDE_DIR** (path) : location of the libobs subfolder in the source code of OBS Studio
- **LIBOBS_LIB** (filepath) : location of the obs.lib file
- **OBS_FRONTEND_LIB** (filepath) : location of the obs-frontend-api.lib file

### Linux
On Debian/Ubuntu :  
```
sudo apt-get install libqt5websockets5-dev
git clone --recursive https://github.com/Palakis/obs-websocket.git
cd obs-websocket
mkdir build && cd build
cmake -DLIBOBS_INCLUDE_DIR="<path to the libobs sub-folder in obs-studio's source code>" ..
make
sudo cp obs-websocket.so /usr/lib/obs-plugins
sudo cp -r ../data /usr/share/obs/obs-plugins/obs-websocket
```

### OS X
*To do*
