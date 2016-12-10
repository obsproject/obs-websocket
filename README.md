obs-websocket
==============
Websocket API for OBS Studio.

## Downloads
Binaries for Windows and Linux are available in the [Releases](https://github.com/Palakis/obs-websocket/releases) section.

## Using obs-websocket
You may want to protect the Websocket server with some form of authentication. To do this, open the "Websocket server settings" dialog under OBS' "Tools" menu. In the settings dialogs, you can enable or disable authentication and set a password for it.

An HTML5 frontend made by [t2t2](https://github.com/t2t2/obs-tablet-remote) (compatible with tablets and other touch interfaces) is available here : http://t2t2.github.io/obs-tablet-remote/

### For developers
The Websocket API server runs on port 4444 and the protocol is documented in [PROTOCOL.md](PROTOCOL.md).  

Here's a list of available language APIs for obs-websocket :
- Javascript (browser & nodejs) : [obs-websocket-js](https://github.com/haganbmj/obs-websocket-js) by haganbmj

I'd like to know what you're building with or for obs-websocket. If you do something in this fashion, feel free to drop me an email at `contact at slepin dot fr` !

### Possible use cases
- Remote control OBS from a phone or tablet on the same local network
- Change your stream overlay/graphics based on the current scene (like the AGDQ overlay does)
- Automate scene switching with a third-party program (e.g. : auto-pilot, foot pedal, ...)

## Compiling obs-websocket
### Prerequisites
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
cmake -DLIBOBS_INCLUDE_DIR="<path to the libobs sub-folder in obs-studio's source code>" -DCMAKE_INSTALL_PREFIX=/usr ..
make -j4
sudo make install
```

### OS X
*To do*
