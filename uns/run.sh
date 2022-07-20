#!/bin/sh
LINES=`wc -l src/*.cpp src/*.h  | grep -o "^\s\(.*\)total" | grep -o "[0-9]*"`
echo "COMPILING $LINES LINES OF CODE"
qmake uns.pro
make
sudo daemon --name=uns --stop
sudo killall -q uns
sudo cp uns_settings /etc/uns_settings
sudo cp bin/uns /usr/local/bin/uns
sudo /usr/local/bin/uns