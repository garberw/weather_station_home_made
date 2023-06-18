#!/bin/bash

pppc=/home/garberw/Arduino/weather_bug

sssc=/dev/ttyACMwa

# this is not working:

# for example
# $pppc/sketch.json
# contains defaults for
# --port (-p) 
# --fqbn (-b) = arduino:avr:uno

function build() {

    echo "-------- build $1 --------"
    
    # arduino-cli compile --fqbn arduino:avr:uno weather_client
    arduino-cli compile $1
    res=$?
    
    echo "res=$res"
    
    if [[ "${?}y" == "0y" ]]; then
	echo "-------- upload $1 port $2 --------"
	echo "does $2 exist?"
	ls $2
	
	# arduino-cli upload --port /dev/ttyUSB0 --fqbn arduino:avr:uno weather_client
	arduino-cli upload  --port $2  $1
	res=$?

	echo "res=$res"
    fi
}

build $pppc $sssc


# eee eof
