obs-websocket
==============
Websocket API for OBS Studio.

## Build prerequisites
You need QT 5.7 (with QtWebSockets), CMake, and a working development environment for OBS Studio installed on your computer.

## How to build
In CMake, you'll need to fill these CMake variables :
- **QTDIR** (path) : location of the Qt environment suited for your compiler and architecture
- **LIBOBS_INCLUDE_DIR** (path) : location of the libobs subfolder in the source code of OBS Studio
- **LIBOBS_LIB** (filepath) : location of the obs.lib file
- **OBS_FRONTEND_LIB** (filepath) : location of the obs-frontend-api.lib file

After building the obs-websocket plugin's binary, copy its Qt dependencies (QtCore, QtNetwork and QtWebSockets library binaries) in the same folder.

## How to use
There's currently no frontend or language API available for obs-websocket. However, the full protocol reference is documented in the [PROTOCOL.md](PROTOCOL.md) file.  
A simple websocket client can connect to the plugin's embedded server.