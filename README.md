obs-websocket
==============
Remote control of OBS Studio made easy.

[![Build Status - Windows](https://ci.appveyor.com/api/projects/status/github/Palakis/obs-websocket)](https://ci.appveyor.com/project/Palakis/obs-websocket/history) [![Build Status - Linux & OS X](https://travis-ci.org/Palakis/obs-websocket.svg?branch=master)](https://travis-ci.org/Palakis/obs-websocket)

## Downloads
Binaries for Windows, macOS and Linux are available in the [Releases](https://github.com/Palakis/obs-websocket/releases) section.

## Using obs-websocket
A web client and frontend made by [t2t2](https://github.com/t2t2/obs-tablet-remote) (compatible with tablets and other touch interfaces) is available here : http://t2t2.github.io/obs-tablet-remote/

It is **highly recommended** to protect obs-websocket with a password against unauthorized control. To do this, open the "Websocket server settings" dialog under OBS' "Tools" menu. In the settings dialogs, you can enable or disable authentication and set a password for it.

### Possible use cases
- Remote control OBS from a phone or tablet on the same local network
- Change your stream overlay/graphics based on the current scene
- Automate scene switching with a third-party program (e.g. : auto-pilot, foot pedal, ...)

### For developers
The server is a typical WebSockets server running by default on port 4444 (the port number can be changed in the Settings dialog). 
The protocol understood by the server is documented in [PROTOCOL.md](docs/generated/protocol.md).  

Here's a list of available language APIs for obs-websocket :
- Javascript (browser & nodejs): [obs-websocket-js](https://github.com/haganbmj/obs-websocket-js) by Brendan Hagan
- C#/VB.NET: [obs-websocket-dotnet](https://github.com/Palakis/obs-websocket-dotnet)
- Python 2 and 3: [obs-websocket-py](https://github.com/Elektordi/obs-websocket-py) by Guillaume Genty a.k.a Elektordi
- Python 3.5+ with asyncio: [obs-ws-rc](https://github.com/KirillMysnik/obs-ws-rc) by Kirill Mysnik

I'd like to know what you're building with or for obs-websocket. If you do something in this fashion, feel free to drop me an email at `contact at slepin dot fr` !

## Compiling obs-websocket
See the [build instructions](BUILDING.md).

## Translations
**We need your help on translations**. Please join the localization project on Crowdin: https://crowdin.com/project/obs-websocket

## Special thanks
In order of appearance:
- [Brendan H.](https://github.com/haganbmj) : Code contributions and gooder English in the Protocol specification
- [Mikhail Swift](https://github.com/mikhailswift) : Code contributions
- [Tobias Frahmer](https://github.com/Frahmer) : German localization
- [Genture](https://github.com/Genteure) : Simplified Chinese and Traditional Chinese localizations
- [Larissa Gabilan](https://github.com/laris151) : Portuguese localization
- [Andy Asquelt](https://github.com/asquelt) : Polish localization
- [Marcel Haazen](https://github.com/inpothet) : Dutch localization
- [Peter Antonvich](https://github.com/pantonvich) : Code contributions
- [yinzara](https://github.com/yinzara) : Code contributions
- [Chris Angelico](https://github.com/Rosuav) : Code contributions
- [Guillaume "Elektordi" Genty](https://github.com/Elektordi) : Code contributions
- [Marwin M](https://github.com/dragonbane0) : Code contributions
- [Logan S.](https://github.com/lsdaniel) : Code contributions
- [RainbowEK](https://github.com/RainbowEK) : Code contributions
- [RytoEX](https://github.com/RytoEX) : CI script and code contributions
- [Theodore Stoddard](https://github.com/TStod) : Code contributions
- [Philip Loche](https://github.com/PicoCentauri) : Code contributions
- Everyone contributing localizations on Crowdin!

And also: special thanks to supporters of the project!

## Supporters
They have contributed financially to the project and made possible the addition of several features into obs-websocket. Many thanks to them!

---

[Support Class](http://supportclass.net) designs and develops professional livestreams, with services ranging from broadcast graphics design and integration to event organization, along many other skills.  

[![Support Class](.github/images/supportclass_logo_blacktext.png)](http://supportclass.net)

---

[MediaUnit](http://www.mediaunit.no) is a Norwegian media company developing products and services for the media industry, primarly focused on web and events.  

[![MediaUnit](.github/images/mediaunit_logo_black.png)](http://www.mediaunit.no/)
