#!/bin/bash

# for desktop and raspi; for root and user;
# backup ~/.bashrc
# replace ~/.bashrc with copy from git;

# set -x

# exit when any command does not return 0
set -e
source /etc/default/wshm
source $GIT_HOME/HUB/CONFIG

checkhost $DESKTOP_HOST

checkroot



function raspi_setup() {
    TARGET_HOST="weather${1}"
    LOC="on TARGET_HOST=$TARGET_HOST"

    echo "$PROMPT $LOC install ~/.bashrc.orig (original version from /etc/skel) (for root, user)"
    scp $GIT_HOME/raspi_root/hidden/dotbashrc.orig $TARGET_HOST:$ROOT_HOME/.bashrc.orig
    ssh $TARGET_HOST chown root.root $ROOT_HOME/.bashrc.orig
    scp $GIT_HOME/raspi_user/hidden/dotbashrc.orig $TARGET_HOST:$USER_HOME/.bashrc.orig

    echo "$PROMPT $LOC backup ~/.bashrc with date (for root, user)"
    ssh $TARGET_HOST cp -f $ROOT_HOME/.bashrc $ROOT_HOME/.bashrc.`date -Is`
    ssh $TARGET_HOST chown root.root $ROOT_HOME/.bashrc.*
    ssh $TARGET_HOST cp -af $USER_HOME/.bashrc $USER_HOME/.bashrc.`date -Is`

    echo "$PROMPT $LOC install $GIT_HOME/.bashrc  (replace original) (for root, user)"
    scp $GIT_HOME/raspi_root/hidden/dotbashrc${1} $TARGET_HOST:$ROOT_HOME/.bashrc
    ssh $TARGET_HOST chown root.root $ROOT_HOME/.bashrc
    scp $GIT_HOME/raspi_user/hidden/dotbashrc${1} $TARGET_HOST:$USER_HOME/.bashrc
}

LOC="on DESKTOP_HOST=$DESKTOP_HOST"

raspi_setup s
raspi_setup e
raspi_setup g
raspi_setup w

echo "$PROMPT $LOC"
echo "$PROMPT hand edit $GIT_HOME/desktop_hidden/dot.bashrc_weather and install in $USER_HOME"
echo "$PROMPT hand edit $GIT_HOME/desktop_hidden/dot.emacs_weather  and install in $USER_HOME"
echo "$PROMPT consider using $GIT_HOME/raspi_root/hidden/dotemacs* as ~/.emacs"
echo "$PROMPT $LOC and on raspi"

# eee eof
