#!/bin/bash

# set -x

# exit when any command does not return 0
set -e
source /etc/default/wshm
source $GIT_HOME/HUB/CONFIG

checkhost $DESKTOP_HOST

# PUSH !!!!!!!!!!!!!!!!!!!!!! THESE ALL RSYNC FROM HERE (DESKTOP) TO RASPI

function weather_sync() {
    TARGET_HOST="weather${1}"
    LOC="on TARGET_HOST=$TARGET_HOST"

    echo "$PROMPT $LOC push arduino"
    ssh $TARGET_HOST mkdir -p $ARDUINO_HOME
    rsync -avz --delete "${ARDUINO_HOME}/"  "${TARGET_HOST}:${ARDUINO_HOME}/"
    
}


weather_sync s
weather_sync e
weather_sync g
weather_sync w

# PUSH !!!!!!!!!!!!!!!!!!!!!! THESE ALL RSYNC FROM HERE (DESKTOP) TO RASPI

# eee eof
