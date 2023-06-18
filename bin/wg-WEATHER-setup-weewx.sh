#!/bin/bash

# set -x

# exit when any command does not return 0
set -e
source /etc/default/wshm
source $GIT_HOME/HUB/CONFIG

checkhost $DESKTOP_HOST

checkroot

$GIT_HOME/bin/wg-WEATHER-push-git-from-desktop-to-raspi.sh

# remote
ssh weathers $GIT_HOME/bin/wg-WEATHER-remote-setup-weewx.sh


# eee eof
