# obs-websocket

## YOU HAVE STUMBLED UPON THE DEV BRANCH FOR V5.0.0

- You can find the main API sheet here: https://gist.github.com/tt2468/59390b99e7841b28f56dffb3dd622ec9
- You can find the planned requests sheet here: https://docs.google.com/spreadsheets/d/1LfCZrbT8e7cSaKo_TuPDd-CJiptL7RSuo8iE63vMmMs/edit?usp=sharing


<p align="center">
  <img src="/.github/images/obsws_logo.png" width=150 align="center">
</p>

WebSockets API for OBS Studio.

[![Build Status](https://dev.azure.com/Palakis/obs-websocket/_apis/build/status/Palakis.obs-websocket?branchName=4.x-current)](https://dev.azure.com/Palakis/obs-websocket/_build/latest?definitionId=2&branchName=4.x-current)
[![CodeFactor](https://www.codefactor.io/repository/github/palakis/obs-websocket/badge)](https://www.codefactor.io/repository/github/palakis/obs-websocket)
[![Twitter](https://img.shields.io/twitter/url/https/twitter.com/fold_left.svg?style=social&label=Follow%20%40LePalakis)](https://twitter.com/LePalakis)
[![Discord](https://img.shields.io/discord/715691013825364120.svg?label=&logo=discord&logoColor=ffffff&color=7389D8&labelColor=6A7EC2)](https://discord.gg/WBaSQ3A)
[![Financial Contributors on Open Collective](https://opencollective.com/obs-websocket/all/badge.svg?label=financial+contributors)](https://opencollective.com/obs-websocket)

## Downloads

Binaries for Windows, MacOS, and Linux are available in the [Releases](https://github.com/Palakis/obs-websocket/releases) section.

### Homebrew

If you're using MacOS you can use Homebrew for installation as well:

```sh
brew install obs-websocket
```

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
- Java 11+: [obs-java-client](https://github.com/harm27/obs-java-client) by harm27
- Golang: [go-obs-websocket](https://github.com/christopher-dG/go-obs-websocket) by Chris de Graaf
- Rust: [obws](https://github.com/dnaka91/obws) by dnaka91
- HTTP API: [obs-websocket-http](https://github.com/IRLToolkit/obs-websocket-http) by tt2468
- CLI: [obs-cli](https://github.com/leafac/obs-cli) by leafac

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

## Contributors

### Code Contributors

This project exists thanks to all the people who contribute. [[Contribute](CONTRIBUTING.md)].
<a href="https://github.com/Palakis/obs-websocket/graphs/contributors"><img src="https://opencollective.com/obs-websocket/contributors.svg?width=890&button=false" /></a>

### Financial Contributors

Become a financial contributor and help us sustain our community. [[Contribute](https://opencollective.com/obs-websocket/contribute)]

#### Individuals

<a href="https://opencollective.com/obs-websocket"><img src="https://opencollective.com/obs-websocket/individuals.svg?width=890"></a>

#### Organizations

Support this project with your organization. Your logo will show up here with a link to your website. [[Contribute](https://opencollective.com/obs-websocket/contribute)]

<a href="https://opencollective.com/obs-websocket/organization/0/website"><img src="https://opencollective.com/obs-websocket/organization/0/avatar.svg"></a>
<a href="https://opencollective.com/obs-websocket/organization/1/website"><img src="https://opencollective.com/obs-websocket/organization/1/avatar.svg"></a>
<a href="https://opencollective.com/obs-websocket/organization/2/website"><img src="https://opencollective.com/obs-websocket/organization/2/avatar.svg"></a>
<a href="https://opencollective.com/obs-websocket/organization/3/website"><img src="https://opencollective.com/obs-websocket/organization/3/avatar.svg"></a>
<a href="https://opencollective.com/obs-websocket/organization/4/website"><img src="https://opencollective.com/obs-websocket/organization/4/avatar.svg"></a>
<a href="https://opencollective.com/obs-websocket/organization/5/website"><img src="https://opencollective.com/obs-websocket/organization/5/avatar.svg"></a>
<a href="https://opencollective.com/obs-websocket/organization/6/website"><img src="https://opencollective.com/obs-websocket/organization/6/avatar.svg"></a>
<a href="https://opencollective.com/obs-websocket/organization/7/website"><img src="https://opencollective.com/obs-websocket/organization/7/avatar.svg"></a>
<a href="https://opencollective.com/obs-websocket/organization/8/website"><img src="https://opencollective.com/obs-websocket/organization/8/avatar.svg"></a>
<a href="https://opencollective.com/obs-websocket/organization/9/website"><img src="https://opencollective.com/obs-websocket/organization/9/avatar.svg"></a>
