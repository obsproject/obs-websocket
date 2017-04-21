obs-websocket
==============
Remote control of OBS Studio made easy.

Follow the project on Twitter for news & updates : [@obswebsocket](https://twitter.com/obswebsocket)

[![Gitter chat](https://badges.gitter.im/obs-websocket/obs-websocket.png)](https://gitter.im/obs-websocket/obs-websocket) [![Build Status - Windows](https://ci.appveyor.com/api/projects/status/github/Palakis/obs-websocket)](https://ci.appveyor.com/project/Palakis/obs-websocket/history) [![Build Status - Linux & OS X](https://travis-ci.org/Palakis/obs-websocket.svg?branch=master)](https://travis-ci.org/Palakis/obs-websocket)

## Downloads
Binaries for Windows and Linux are available in the [Releases](https://github.com/Palakis/obs-websocket/releases) section.

## Using obs-websocket
A web client and frontend made by [t2t2](https://github.com/t2t2/obs-tablet-remote) (compatible with tablets and other touch interfaces) is available here : http://t2t2.github.io/obs-tablet-remote/

It is **highly recommended** to protect obs-websocket with a password against unauthorized control. To do this, open the "Websocket server settings" dialog under OBS' "Tools" menu. In the settings dialogs, you can enable or disable authentication and set a password for it.

### Possible use cases
- Remote control OBS from a phone or tablet on the same local network
- Change your stream overlay/graphics based on the current scene (like the AGDQ overlay does)
- Automate scene switching with a third-party program (e.g. : auto-pilot, foot pedal, ...)

### For developers
The server is a typical Websockets server running by default on port 4444 (the port number can be changed in the Settings dialog). 
The protocol understood by the server is documented in [PROTOCOL.md](PROTOCOL.md).  

Here's a list of available language APIs for obs-websocket :
- Javascript (browser & nodejs) : [obs-websocket-js](https://github.com/haganbmj/obs-websocket-js) by haganbmj
- C#/VB.NET : [obs-websocket-dotnet](https://github.com/Palakis/obs-websocket-dotnet)

I'd like to know what you're building with or for obs-websocket. If you do something in this fashion, feel free to drop me an email at `contact at slepin dot fr` !

## Compiling obs-websocket
See the [build instructions](BUILDING.md).

## Special thanks
- [Brendan H.](https://github.com/haganbmj) : Code contributions and better English in the Protocol specification
- [Mikhail Swift](https://github.com/mikhailswift) : Code contributions
- [Tobias Frahmer](https://github.com/Frahmer) : German translation
- [Genture](https://github.com/Genteure) : Simplified Chinese and Traditional Chinese translations
- [Larissa Gabilan](https://github.com/laris151) : Portuguese translation
- [Andy Asquelt](https://github.com/asquelt) : Polish translation
- [Marcel Haazen](https://github.com/inpothet) : Dutch translation
- Supporters of the project

## Supporters
They have contributed financially to the project and made possible the addition of several features into obs-websocket. Many thanks to them!

---

[Support Class](http://supportclass.net) designs and develops professional livestreams, with services ranging from broadcast graphics design and integration to event organization, along many other skills.  

[![Support Class](doc/supportclass_logo_blacktext.png)](http://supportclass.net)

---

[MediaUnit](http://www.mediaunit.no) is a Norwegian media company developing products and services for the media industry, primarly focused on web and events.  

[![MediaUnit](doc/mediaunit_logo_black.png)](http://www.mediaunit.no/)
