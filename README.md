# obs-websocket

## YOU HAVE STUMBLED UPON THE DEV BRANCH FOR V5.0.0

- You can find the main protocol spec here: [PROTOCOL.md](docs/generated/protocol.md).
- You can find the planned requests sheet [here](https://docs.google.com/spreadsheets/d/1LfCZrbT8e7cSaKo_TuPDd-CJiptL7RSuo8iE63vMmMs/edit?usp=sharing)


<p align="center">
  <img src="/.github/images/obsws_logo.png" width=150 align="center">
</p>

WebSockets API for OBS Studio.

[![CI Multiplatform Build](https://github.com/Palakis/obs-websocket/actions/workflows/main.yml/badge.svg?branch=master)](https://github.com/Palakis/obs-websocket/actions/workflows/main.yml)
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
- (No known clients supporting 5.0.0)

It is **highly recommended** to protect obs-websocket with a password against unauthorized control. To do this, open the "Websocket server settings" dialog under OBS' "Tools" menu. In the settings dialogs, you can enable or disable authentication and set a password for it.

(Psst. You can use `--websocket_port`(value), `--websocket_password`(value), and `--websocket_debug`(flag) on the command line to override the configured values.)

### Possible use cases

- Remote control OBS from a phone or tablet on the same local network
- Change your stream overlay/graphics based on the current scene
- Automate scene switching with a third-party program (e.g. : auto-pilot, foot pedal, ...)

### For developers

The server is a typical Websockets server running by default on port 4444 (the port number can be changed in the Settings dialog).
The protocol understood by the server is documented in [PROTOCOL.md](docs/generated/protocol.md).

Here's a list of available language APIs for obs-websocket :
- Python 3.7+ (Asyncio): [simpleobsws](https://github.com/IRLToolkit/simpleobsws/tree/master) by IRLToolkit

We'd like to know what you're building with or for obs-websocket. If you do something in this fashion, feel free to drop a message in `#project-showoff` in the [discord server!](https://discord.gg/WBaSQ3A)

### Securing obs-websocket (via TLS/SSL)

If you are intending to use obs-websocket outside of a LAN environment, it is highly recommended to secure the connection using a tunneling service.

See the SSL [tunnelling guide](SSL-TUNNELLING.md) for easy instructions on how to encrypt your websocket connection.

## Compiling obs-websocket

See the [build instructions](BUILDING.md).

## Translations

**Your help is welcome on translations.**

Please join the localization project on [Crowdin](https://crowdin.com/project/obs-websocket)

## Contributors

### Code Contributors

This project exists thanks to all the people who contribute. [Contribute](CONTRIBUTING.md).
<a href="https://github.com/Palakis/obs-websocket/graphs/contributors"><img src="https://opencollective.com/obs-websocket/contributors.svg?width=890&button=false" /></a>

### Financial Contributors

Become a financial contributor and help us sustain our community. [Contribute](https://opencollective.com/obs-websocket/contribute)

#### Individuals

<a href="https://opencollective.com/obs-websocket"><img src="https://opencollective.com/obs-websocket/individuals.svg?width=890"></a>

#### Organizations

Support this project with your organization. Your logo will show up here with a link to your website. [Contribute](https://opencollective.com/obs-websocket/contribute)

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
