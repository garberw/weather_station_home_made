#!/bin/bash

# set -x

# exit when any command does not return 0
set -e
source /etc/default/wshm
source $GIT_HOME/HUB/CONFIG

checkhost $DESKTOP_HOST

checkroot

LOC="on DESKTOP_HOST=$DESKTOP_HOST"

echo "$PROMPT $LOC"
echo "$PROMPT user's udev devices will not be same as git example udev devices;"
echo "$PROMPT when you get a new arduino"
echo "$PROMPT (1) run udev 'get' script in udev dir;"
echo "$PROMPT (2) update by hand the git udev devices;"
echo "$PROMPT (3) do not overwrite hand edit with value from git repository;"
echo "$PROMPT (4) run udev_setup.sh script;"
echo "$PROMPT this script does (4) to install on each raspi;"

echo -n "$PROMPT enter to continue > "
read answer

echo "$PROMPT on desktop and each raspi setup udev"
$GIT_HOME/WEEWX/udev/udev_setup.sh
ssh weathers $GIT_HOME/WEEWX/udev/udev_setup.sh
ssh weathere $GIT_HOME/WEEWX/udev/udev_setup.sh
ssh weatherg $GIT_HOME/WEEWX/udev/udev_setup.sh
ssh weatherw $GIT_HOME/WEEWX/udev/udev_setup.sh

# eee eof
