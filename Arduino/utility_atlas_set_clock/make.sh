#!/bin/bash

pppa=/home/garberw/Arduino/weather_atlas_set_clock

SSSa=/dev/ttyACMwa
sssa=$(/usr/bin/readlink -f $SSSa)

# this is not working:

# for example
# $pppa/sketch.json
# contains defaults for
# --port (-p) 
# --fqbn (-b) = arduino:avr:uno
# --fqbn (-b) = adafruit:samd:adafruit_itsybitsy_m4

function build() {
    echo "-------- build $1 --------"
    # arduino-cli compile --fqbn adafruit:samd:adafruit_itsybitsy_m4 weather_client
    arduino-cli compile $1
    res=$?
    echo "res=$res"
    if [[ "${res}y" == "0y" ]]; then
	echo "-------- upload $1 port $2 = $3 --------"
	if [[ -e "$2" ]]; then
	    echo "$2 exists"
	    # arduino-cli upload --port /dev/ttyACM0 --fqbn adafruit:samd:adafruit_itsybitsy_m4 weather_client
	    arduino-cli upload  --port $2  $1
	    res=$?
	    echo "res=$res"
	else
	    echo "error; $2 does not exist"
	fi
    fi
}

build $pppa $sssa $SSSa

# eee eof
