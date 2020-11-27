# obs-websocket

<p align="center">
  <img src="/.github/images/obsws_logo.png" width=150 align="center">
</p>

WebSockets API for OBS Studio.

[![Discord](https://img.shields.io/discord/715691013825364120.svg?label=&logo=discord&logoColor=ffffff&color=7389D8&labelColor=6A7EC2)](https://discord.gg/WBaSQ3A)
[![Build Status](https://dev.azure.com/Palakis/obs-websocket/_apis/build/status/Palakis.obs-websocket?branchName=4.x-current)](https://dev.azure.com/Palakis/obs-websocket/_build/latest?definitionId=2&branchName=4.x-current)
[![Twitter](https://img.shields.io/twitter/url/https/twitter.com/fold_left.svg?style=social&label=Follow%20%40LePalakis)](https://twitter.com/LePalakis)

## Downloads

Binaries for Windows, MacOS, and Linux are available in the [Releases](https://github.com/Palakis/obs-websocket/releases) section.

## Using obs-websocket

Here is a list of available web clients: (compatible with tablets and other touch interfaces)

- [Niek/obs-web](https://github.com/Niek/obs-web)
- [t2t2/obs-tablet-remote](https://github.com/t2t2/obs-tablet-remote)

It is **highly recommended** to protect obs-websocket with a password against unauthorized control. To do this, open the "Websocket server settings" dialog under OBS' "Tools" menu. In the settings dialogs, you can enable or disable authentication and set a password for it.

### Possible use cases

- Remote control OBS from a phone or tablet on the same local network
- Change your stream overlay/graphics based on the current scene
- Automate scene switching with a third-party program (e.g. : auto-pilot, foot pedal, ...)

### For developers

The server is a typical Websockets server running by default on port 4444 (the port number can be changed in the Settings dialog).
The protocol understood by the server is documented in [PROTOCOL.md](docs/generated/protocol.md).

Here's a list of available language APIs for obs-websocket :
- Javascript (browser & nodejs): [obs-websocket-js](https://github.com/haganbmj/obs-websocket-js) by Brendan Hagan
- C#/VB.NET: [obs-websocket-dotnet](https://github.com/Palakis/obs-websocket-dotnet)
- Python 2 and 3: [obs-websocket-py](https://github.com/Elektordi/obs-websocket-py) by Guillaume Genty a.k.a Elektordi
- Python 3.5+ with asyncio: [obs-ws-rc](https://github.com/KirillMysnik/obs-ws-rc) by Kirill Mysnik
- Python 3.6+ with asyncio: [simpleobsws](https://github.com/IRLToolkit/simpleobsws) by tt2468 
- Java 8+: [obs-websocket-java](https://github.com/Twasi/websocket-obs-java) by TwasiNET
- Golang: [go-obs-websocket](https://github.com/christopher-dG/go-obs-websocket) by Chris de Graaf
- HTTP API: [obs-websocket-http](https://github.com/IRLToolkit/obs-websocket-http) by tt2468

I'd like to know what you're building with or for obs-websocket. If you do something in this fashion, feel free to drop a message in `#project-showoff` in the [discord server!](https://discord.gg/WBaSQ3A)

### Securing obs-websocket (via TLS/SSL)

If you are intending to use obs-websocket outside of a LAN environment, it is highly recommended to secure the connection using a tunneling service.

See the SSL [tunnelling guide](SSL-TUNNELLING.md) for easy instructions on how to encrypt your websocket connection.

## Compiling obs-websocket

See the [build instructions](BUILDING.md).

## Contributing

See [the contributing document](/CONTRIBUTING.md)

## Translations

**Your help is welcome on translations.**

Please join the localization project on [Crowdin](https://crowdin.com/project/obs-websocket)

## Special thanks

Thank you so much to all of the contibutors [(here)](https://github.com/Palakis/obs-websocket/graphs/contributors) for your amazing help.

And also: special thanks to supporters of the project!

## Supporters

These supporters have contributed financially to the project and made possible the addition of several features into obs-websocket. Many thanks to them!

---

[Support Class](http://supportclass.net) designs and develops professional livestreams, with services ranging from broadcast graphics design and integration to event organization, along many other skills.  

[![Support Class](.github/images/supportclass_logo_blacktext.png)](http://supportclass.net)

---

[MediaUnit](http://www.mediaunit.no) is a Norwegian media company developing products and services for the media industry, primarly focused on web and events.  

[![MediaUnit](.github/images/mediaunit_logo_black.png)](http://www.mediaunit.no/)
