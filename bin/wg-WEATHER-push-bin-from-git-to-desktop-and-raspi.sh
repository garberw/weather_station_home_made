#!/bin/bash

# for desktop and raspi; for root and user;
# copy from git to ~/bin

# set -x

# exit when any command does not return 0
set -e
source /etc/default/wshm
source $GIT_HOME/HUB/CONFIG

checkhost $DESKTOP_HOST

checkroot

# this does not assume git is up to date on TARGET_HOST;
# independent of wg-WEATHER-push-git-from-desktop-to-raspi;
function raspi_setup() {
    TARGET_HOST="weather${1}"
    LOC="on TARGET_HOST=$TARGET_HOST"

    echo "$PROMPT $LOC install $GIT_HOME/bin scripts to ~/bin (for root, user)"
    scp $GIT_HOME/bin/ALIAS-ARDUINO $TARGET_HOST:$ROOT_HOME/bin/
    ssh $TARGET_HOST chown root:root $ROOT_HOME/bin/ALIAS-ARDUINO
    scp $GIT_HOME/bin/ALIAS-ARDUINO $TARGET_HOST:$USER_HOME/bin/
    # omit bak; leave other existing target files intact;
    scp $GIT_HOME/bin/wg-* $TARGET_HOST:$ROOT_HOME/bin/
    scp $GIT_HOME/bin/wg-* $TARGET_HOST:$USER_HOME/bin/
    # this does not work because glob is on desktop
    # ssh $TARGET_HOST chown root:root $ROOT_HOME/bin/wg-*
    ssh $TARGET_HOST chown -R root:root $ROOT_HOME/bin
}

LOC="on DESKTOP_HOST=$DESKTOP_HOST"

echo "$PROMPT $LOC install $GIT_HOME/bin scripts to  ~/bin"
# no --delete
cp -af $GIT_HOME/bin/ALIAS-ARDUINO $USER_HOME/bin/
cp -af $GIT_HOME/bin/wg-*          $USER_HOME/bin/
cp -af $GIT_HOME/bin/ALIAS-ARDUINO $ROOT_HOME/bin/
cp -af $GIT_HOME/bin/wg-*          $ROOT_HOME/bin/
chown root:root $ROOT_HOME/bin/ALIAS-ARDUINO
chown root:root $ROOT_HOME/bin/wg-arduino-*
chown root:root $ROOT_HOME/bin/wg-WEATHER-*

raspi_setup s
raspi_setup e
raspi_setup g
raspi_setup w


# eee eof
