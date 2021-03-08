# Connecting over a TLS/secure connection (or remotely)

If you want to expose the WebSocket server of obs-websocket over a secure TLS connection (or to connect remotely), the easiest approach is to use a localhost tunneling service like [ngrok](https://ngrok.com/) or [pagekite](https://pagekite.net/).

**Before doing this, secure the WebSocket server first by enabling authentication with a strong password!**

**Please bear in mind that doing this will expose your OBS instance to the open Internet and the security risks it implies. *You've been warned!***


## ngrok

[Install the ngrok CLI tool](https://ngrok.com/download) on a linux OS, then start ngrok bound to port 4444 like this:

```bash
ngrok http 4444
```

The ngrok command will output something like this:

```text
ngrok by @inconshreveable

Tunnel Status                 online
Version                       2.0/2.0
Web Interface                 http://127.0.0.1:4040
Forwarding                    http://TUNNEL_ID.ngrok.io -> localhost:4444
Forwarding                    https://TUNNEL_ID.ngrok.io -> localhost:4444
```

Where `TUNNEL_ID` is, as the name implies, the unique name of your ngrok tunnel. You'll get a new one every time you start ngrok.

Then, use `wss://TUNNEL_ID.ngrok.io` to connect to obs-websocket over TLS.

See the [ngrok documentation](https://ngrok.com/docs) for more tunneling options and settings.


## PageKite

[Install the PageKite CLI tool](http://pagekite.net/downloads), then start PageKite bound to port 4444 like this (replace NAME with one of your choosing):

```bash
python pagekite.py 4444 NAME.pagekite.me
```

Then, use `wss://NAME.pagekite.me` to connect to obs-websocket over TLS.
