#!/bin/bash

USER_HOME="/home/garberw/"
GIT_HOME="${USER_HOME}/git/weather_station_home_made/"

SRC="$GIT_HOME/flags-arduino"
DST="$USER_HOME/.arduino15/packages/adafruit/hardware/samd/1.7.11/"

cp "${SRC}/platform.local.txt" "$DST"

# cp "${SRC}/platform.txt" "$DST"


# eee eof
