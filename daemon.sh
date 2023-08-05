#!/bin/sh
sudo daemon --name=uns --stop
sudo killall -q uns
sudo daemon --name=uns --user=root --unsafe --respawn --errlog=/var/log/uns.log --dbglog=/var/log/uns.log --output=/var/log/uns.log --stdout=/var/log/uns.log --stderr=/var/log/uns.log /usr/local/bin/uns