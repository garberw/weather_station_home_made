#!/bin/bash

# set -x

# exit when any command does not return 0
set -e
source /etc/default/wshm
source $GIT_HOME/HUB/CONFIG

checkhost $DESKTOP_HOST

checkroot

# THESE ALL RSYNC TO HERE (GET, PULL) !!!!!!!!!!!!!!!!!!!!!!!!!! =================

rsync -avz --delete weathers:/var/lib/weewx/ /var/lib/weewx/

# THESE ALL RSYNC TO HERE (GET, PULL) !!!!!!!!!!!!!!!!!!!!!!!!!! =================


echo
echo -n "copy to junk (backup to temporary necessary for wg-weather-gui.py) ? enter or CTRL-c > "
read answer

# change directory
cd /var/lib/weewx/
./wg-copy-to-junk.sh

# eee eof
