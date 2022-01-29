# obs-websocket

<p align="center">
  <img src="/.github/images/obsws_logo.png" width=150 align="center">
</p>

WebSocket API for OBS Studio.

[![CI Multiplatform Build](https://github.com/obsproject/obs-websocket/actions/workflows/main.yml/badge.svg?branch=master)](https://github.com/obsproject/obs-websocket/actions/workflows/main.yml)
[![Discord](https://img.shields.io/discord/715691013825364120.svg?label=&logo=discord&logoColor=ffffff&color=7389D8&labelColor=6A7EC2)](https://discord.gg/WBaSQ3A)
[![Financial Contributors on Open Collective](https://opencollective.com/obs-websocket-dev/all/badge.svg?label=financial+contributors)](https://opencollective.com/obs-websocket-dev)

## Downloads

Binaries for Windows, MacOS, and Linux are available in the [Releases](https://github.com/obsproject/obs-websocket/releases) section.

## Using obs-websocket

It is **highly recommended** to protect obs-websocket with a password against unauthorized control. To do this, open the "Websocket server settings" dialog under OBS' "Tools" menu. In the settings dialogs, you can enable or disable authentication and set a password for it.

(Psst. You can use `--websocket_port`(value), `--websocket_password`(value), and `--websocket_debug`(flag) on the command line to override the configured values.)

### Possible use cases

- Remote control OBS from a phone or tablet on the same local network
- Change your stream overlay/graphics based on the current scene
- Automate scene switching with a third-party program (e.g. : auto-pilot, foot pedal, ...)

### Client software
- (No known clients supporting 5.0.0 at the moment. Ping us in the Discord if you have one!)

### Client libraries (for developers)

Here's a list of available language APIs for obs-websocket:
- Python 3.7+ (Asyncio): [simpleobsws](https://github.com/IRLToolkit/simpleobsws/tree/master) by IRLToolkit
- Rust: [obws](https://github.com/dnaka91/obws/tree/v5-api) by dnaka91

The 5.x server is a typical WebSocket server running by default on port 4455 (the port number can be changed in the Settings dialog under `Tools`).
The protocol we use is documented in [PROTOCOL.md](docs/generated/protocol.md).

We'd like to know what you're building with obs-websocket! If you do something in this fashion, feel free to drop a message in `#project-showoff` in the [discord server!](https://discord.gg/WBaSQ3A)

## Contributors

### Code Contributors

This project exists thanks to [all the people](graphs/contributors) who contribute. [Contribute](wiki/Contributing-Guidelines).
<a href="https://github.com/obsproject/obs-websocket/graphs/contributors"><img src="https://opencollective.com/obs-websocket-dev/contributors.svg?width=890&button=false" /></a>

### Financial Contributors

Become a financial contributor and help us sustain our community. [Contribute](https://opencollective.com/obs-websocket-dev/contribute)

#### Individuals

<a href="https://opencollective.com/obs-websocket-dev"><img src="https://opencollective.com/obs-websocket-dev/individuals.svg?width=890"></a>

#### Organizations

Support this project with your organization. Your logo will show up here with a link to your website. [Contribute](https://opencollective.com/obs-websocket-dev/contribute)

<a href="https://opencollective.com/obs-websocket-dev/organization/0/website"><img src="https://opencollective.com/obs-websocket-dev/organization/0/avatar.svg"></a>
<a href="https://opencollective.com/obs-websocket-dev/organization/1/website"><img src="https://opencollective.com/obs-websocket-dev/organization/1/avatar.svg"></a>
<a href="https://opencollective.com/obs-websocket-dev/organization/2/website"><img src="https://opencollective.com/obs-websocket-dev/organization/2/avatar.svg"></a>
<a href="https://opencollective.com/obs-websocket-dev/organization/3/website"><img src="https://opencollective.com/obs-websocket-dev/organization/3/avatar.svg"></a>
<a href="https://opencollective.com/obs-websocket-dev/organization/4/website"><img src="https://opencollective.com/obs-websocket-dev/organization/4/avatar.svg"></a>
<a href="https://opencollective.com/obs-websocket-dev/organization/5/website"><img src="https://opencollective.com/obs-websocket-dev/organization/5/avatar.svg"></a>
<a href="https://opencollective.com/obs-websocket-dev/organization/6/website"><img src="https://opencollective.com/obs-websocket-dev/organization/6/avatar.svg"></a>
<a href="https://opencollective.com/obs-websocket-dev/organization/7/website"><img src="https://opencollective.com/obs-websocket-dev/organization/7/avatar.svg"></a>
<a href="https://opencollective.com/obs-websocket-dev/organization/8/website"><img src="https://opencollective.com/obs-websocket-dev/organization/8/avatar.svg"></a>
<a href="https://opencollective.com/obs-websocket-dev/organization/9/website"><img src="https://opencollective.com/obs-websocket-dev/organization/9/avatar.svg"></a>
