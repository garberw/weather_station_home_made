#!/bin/bash

ppps=/home/garberw/Arduino/test_light_server

SSSs=/dev/ttyACMwl
ssss=$(/usr/bin/readlink -f $SSSs)

# this is not working:

# for example
# $pppc/sketch.json
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

build $ppps $ssss $SSSs

# eee eof
