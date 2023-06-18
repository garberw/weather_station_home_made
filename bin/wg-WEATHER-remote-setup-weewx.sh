#!/bin/bash

# set -x

# exit when any command does not return 0
set -e
source /etc/default/wshm
source $GIT_HOME/HUB/CONFIG

# note weathers not desktop
checkhost weathers

checkroot

echo "$PROMPT install to /etc/default"
cp -f $GIT_HOME/WEEWX/Etc_default/weewx_atlas /etc/default
cp -f $GIT_HOME/WEEWX/Etc_default/weewx_green /etc/default
cp -f $GIT_HOME/WEEWX/Etc_default/weewx_merge /etc/default
cp -f $GIT_HOME/WEEWX/Etc_default/weewx_wmod  /etc/default

echo "$PROMPT install /etc/weewx/weewx_*.conf"
cp -f $GIT_HOME/WEEWX/etc_weewx/weewx_atlas.conf /etc/weewx
cp -f $GIT_HOME/WEEWX/etc_weewx/weewx_green.conf /etc/weewx
cp -f $GIT_HOME/WEEWX/etc_weewx/weewx_merge.conf /etc/weewx
cp -f $GIT_HOME/WEEWX/etc_weewx/weewx_wmod.conf  /etc/weewx

echo "$PROMPT install /usr/share/weewx/user/skins"
rsync -av --delete "$GIT_HOME/WEEWX/etc_weewx/skins/" "/usr/share/weewx/user/skins/"

echo "$PROMPT install binaries /usr/bin/wee*"
cp -f $GIT_HOME/WEEWX/SCRIPTS/wee* /usr/bin

echo "$PROMPT install services /etc/systemd/system/weewx_*"
$GIT_HOME/WEEWX/systemd/wg-install.sh

echo "$PROMPT install /usr/share/weewx/user/schema_garberw.py"
cp -f $GIT_HOME/WEEWX/schemas/schema_garberw.py /usr/share/weewx/user
echo "$PROMPT examine and run $GIT_HOME/WEEWX/schemas/wg-change-schema.{00,11}.sh manually"
echo "$PROMPT and make sure it passes before continuing;"
echo "$PROMPT this adds necessary columns to database"
echo "$PROMPT so you should not run it twice !!!!"

echo "$PROMPT install weewx-schema-garberw units extension"
$GIT_HOME/WEEWX/weewx-schema-garberw/wg-do-install.sh
echo "$PROMPT run $GIT_HOME/WEEWX/weewx-schema-garberw/wg-do-test.sh after this script"

echo "$PROMPT install weewx-atlas service (not driver) extension for atlas indoors"
$GIT_HOME/WEEWX/weewx-atlas/wg-do-install.sh
echo "$PROMPT run $GIT_HOME/WEEWX/weewx-atlas/wg-do-TEST.sh after this script"

echo "$PROMPT install weewx-sdr driver extension for atlas outdoors"
$GIT_HOME/WEEWX/weewx-sdr/wg-do-install.sh
echo "$PROMPT run tests $GIT_HOME/WEEWX/weewx-sdr/wg-4*.sh  and wg-5*.sh after this script"

echo "$PROMPT install weewx-green driver extension for green indoors and outdoors"
$GIT_HOME/WEEWX/weewx-green/wg-do-install.sh
echo "$PROMPT run $GIT_HOME/WEEWX/weewx-green/wg-do-test.sh after this script"

echo "$PROMPT install weewx-wmod driver extension for wmod indoors and outdoors"
$GIT_HOME/WEEWX/weewx-wmod/wg-do-install.sh
echo "$PROMPT run $GIT_HOME/WEEWX/weewx-wmod/wg-do-test.sh after this script"



# eee eof
