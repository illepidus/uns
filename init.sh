#!/bin/bash

# fail on first error
set -e

echo $SUDO_USER

#check if sudo
if [[ -z $SUDO_USER ]]; then
  echo "run this as sudo" 
  exit 1
fi

#install wiringpi (as it's already deprecated and excluded from bull raspbian bullseye)
wget -nc https://github.com/WiringPi/WiringPi/releases/download/2.61-1/wiringpi-2.61-1-armhf.deb
chown $SUDO_USER wiringpi-2.61-1-armhf.deb
dpkg -i wiringpi-2.61-1-armhf.deb

#grant rights
usermod -aG gpio $SUDO_USER
usermod -aG dialout $SUDO_USER

#install packages
apt update
apt upgrade
apt install -y \
  zip \
  daemon \
  wiringpi \
  lighttpd \
  php8.0-dev \
  php8.0-cgi \
  php8.0-curl \
  gcc \
  g++ \
  qtdeclarative5-dev \
  libqt5serialport5-dev \
  qtbase5-dev \
  qtbase5-dev-tools \
  qt5-qmake \
  linux-source \
  make
