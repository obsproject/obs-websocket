#!/bin/sh
set -ex

echo "[obs-websocket] Installing obs-studio PPA and updates.."
sudo add-apt-repository -y ppa:obsproject/obs-studio
sudo apt-get -qq update

echo "[obs-websocket] Installing obs-studio and dependencies.."
sudo apt-get install -y \
	libc-dev-bin \
	libc6-dev git \
	build-essential \
	checkinstall \
	cmake \
	obs-studio \
	qtbase5-dev

echo "[obs-websocket] Installed OBS Version: $(obs --version)"

ls /usr/include/
ls /usr/include/obs/

# Dirty hack
sudo wget -O /usr/include/obs/obs-frontend-api.h https://raw.githubusercontent.com/obsproject/obs-studio/27.1.3/UI/obs-frontend-api/obs-frontend-api.h

sudo ldconfig
