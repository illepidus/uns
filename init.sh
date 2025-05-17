#!/bin/bash

# fail on first error
# set -e

#install wiringpi (as it's already deprecated and excluded from bull raspbian bullseye)
wget -nc https://github.com/WiringPi/WiringPi/releases/download/2.61-1/wiringpi-2.61-1-armhf.deb
sudo dpkg -i wiringpi-2.61-1-armhf.deb

#grant rights
sudo usermod -aG gpio $USER
sudo usermod -aG dialout $USER

#install packages
sudo apt update
sudo apt upgrade
sudo apt install -y \
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

#make
./cleanmake.sh

#Install
sudo killall -q uns
sudo daemon --name=uns --stop
sudo cp uns_settings /etc/uns_settings
sudo cp bin/uns /usr/local/bin/uns

#autostart
sudo cp etc/rc.local /etc/rc.local

#start daemon
./daemon.sh

#web server
sudo cp etc/lighttpd.conf /etc/lighttpd/lighttpd.conf
sudo cp etc/lighttpd-plain.user /etc/lighttpd/lighttpd-plain.user
sudo cp etc/99-uns.rules /etc/udev/rules.d/99-uns.rules

sudo cp -r www/* /var/www
sudo lighty-enable-mod fastcgi 
sudo lighty-enable-mod fastcgi-php
sudo service lighttpd restart

#wireguard
sudo cp etc/wg0.conf /etc/wireguard/wg0.conf
wg-quick up wg0
sudo systemctl enable wg-quick@wg0

echo "Init compleated"
