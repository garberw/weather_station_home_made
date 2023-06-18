#!/bin/bash

BOARD='--fqbn adafruit:samd:adafruit_itsybitsy_m4'
ARDUINO_CLI='/home/garberw/bin/arduino-cli'
HEX_SRC=/tmp/arduino/
CPP_SRC=/home/garberw/Arduino/
ARDUINO_DIR=/home/garberw/.arduino15/
THIS_HOST=electron
RESET_PORTS=/home/garberw/bin/wg-arduino-stty.sh 

sketch_c=/home/garberw/Arduino/weather_dri_client
sketch_l=/home/garberw/Arduino/weather_dri_server_light
sketch_r=/home/garberw/Arduino/weather_dri_server_rain
sketch_w=/home/garberw/Arduino/weather_dri_wmod
sketch_a=/home/garberw/Arduino/weather_dri_atlas
sketch_g=/home/garberw/Arduino/weather_dri_green

port_link_c=/dev/ttyACMwc
port_link_l=/dev/ttyACMwl
port_link_r=/dev/ttyACMwr
port_link_w=/dev/ttyACMww
port_link_a=/dev/ttyACMwa
port_link_g=/dev/ttyACMwg

remote_host_c=weatherw
remote_host_l=weatherw
remote_host_r=weatherw
remote_host_w=weathers
remote_host_a=weathers
remote_host_g=weatherg

# h is for when bootloader not working; redefine all the h variables below
# and symlink /dev/ttyACM[0-3] to /dev/ttyACMwh
# double tap the reset button on the arduino (like double click)
# it should turn green and red
# reprogram it
# when it comes online the udev rule will name the device;
# e.g. it will appear as /dev/ttyACMwl as well as just /dev/ttyACM0 or whatever it was;
# check you programmed it with the right source code; if not redo it;

# sketch_h=/home/garberw/Arduino/weather_dri_green
sketch_h=/home/garberw/Arduino/utility_infineon_single_shot
remote_host_h=weathers
# remote_host_h=weatherg
 port_link_h=/dev/ttyACMww
# port_link_h=/dev/ttyACM0
# port_link_h=/dev/ttyACM1


# $sketch_c/sketch.json contains defaults for
# --port (-p) = /dev/ttyACMwc
# --fqbn (-b) = adafruit:samd:adafruit_itsybitsy_m4

res=1
PORT_DEVICE=""
DESCRIPTION=""

function func_beg() {
    DESCRIPTION="$1"
    echo "-------- $DESCRIPTION --------"
    set -x
}

function func_end() {
    set +x
    res="$1"
    if [[ $res -eq 0 ]]; then
	echo "$DESCRIPTION passed"
    else
	echo "$DESCRIPTION failed; error; exit 1;"
	exit 1
    fi
}

function build() {
    SKETCH="$1"
    PORT_LINK="$2"
    LOAD_HOST="$3"

    # sed chomps trailing whitespace
    # PIPESTATUS[0] = exit code of first  command in pipeline
    # PIPESTATUS[1] = exit code of second command in pipeline
    func_beg "compile"
    $ARDUINO_CLI compile $BOARD $SKETCH | sed 's/ *$//'
    func_end ${PIPESTATUS[0]}

    # fixme no load
    # return
    
    if [[ $LOAD_HOST == $THIS_HOST ]]; then

	func_beg "resolve port"
	PORT_DEVICE=$(/usr/bin/readlink -f $PORT_LINK)
	func_end $?
	
	echo "PORT_LINK   = $PORT_LINK"
	echo "PORT_DEVICE = $PORT_DEVICE"

	func_beg "upload"
	$ARDUINO_CLI upload --port $PORT_DEVICE $BOARD $SKETCH
	func_end $?
	
    else

	func_beg "resolve port"
	PORT_DEVICE=$(ssh $LOAD_HOST /usr/bin/readlink -f $PORT_LINK)
	func_end $?
	
	echo "PORT_LINK   = $PORT_LINK"
	echo "PORT_DEVICE = $PORT_DEVICE"

	# rsync .o / HEX code ======================================================

        func_beg "rsync HEX_SRC"
	rsync -avz --delete $HEX_SRC ${LOAD_HOST}:${HEX_SRC}
	func_end $?

        # rsync config ======================================================

### do not do this
###	func_beg "rsync .arduino15"
###	rsync -avz --delete $ARDUINO_DIR ${LOAD_HOST}:${ARDUINO_DIR}
###	func_end $?

	func_beg "upload"
	ssh $LOAD_HOST $ARDUINO_CLI upload --port $PORT_DEVICE $BOARD $SKETCH
	func_end $?

    fi
}

#########  /home/garberw/bin/wg-arduino-stty.sh

# rsync C++ source code ======================================================

func_beg "rsync weathers"
rsync -avz --delete $CPP_SRC weathers:${CPP_SRC}
func_end $?

func_beg "rsync weatherw"
rsync -avz --delete $CPP_SRC weatherw:${CPP_SRC}
func_end $?

func_beg "rsync weatherg"
rsync -avz --delete $CPP_SRC weatherg:${CPP_SRC}
func_end $?

# rsync C++ source code ======================================================

## $RESET_PORTS
#ssh weathers $RESET_PORTS
#ssh weatherw $RESET_PORTS
#ssh weatherg $RESET_PORTS

# weathere is not involved

if [[ $# -ne 1 ]]; then
    echo "error:  takes one arg;"
    echo "             WMOD"
    echo "c = weather_client       wpc on weatherw remote"
    echo "l = weather_server_light wpl on weatherw remote"
    echo "r = weather_server_rain  wpr on weatherw remote"
    echo "w = weather_wmod         wpw on weathers local"
    echo "             ATLAS"
    echo "a = weather_atlas        wpa on weathers local"
    echo "             GREEN"
    echo "g = weather_green        wpg on weatherg remote"
    echo "x = all"
    echo "m = all WMOD"
    echo "h = debug"
    exit 1
elif [[ "x$1" == "xc" ]]; then
    build "$sketch_c" "$port_link_c" "$remote_host_c"
elif [[ "x$1" == "xl" ]]; then
    build "$sketch_l" "$port_link_l" "$remote_host_l"
elif [[ "x$1" == "xr" ]]; then
    build "$sketch_r" "$port_link_r" "$remote_host_r"
elif [[ "x$1" == "xw" ]]; then
    build "$sketch_w" "$port_link_w" "$remote_host_w"
elif [[ "x$1" == "xa" ]]; then
    build "$sketch_a" "$port_link_a" "$remote_host_a"
elif [[ "x$1" == "xg" ]]; then
    build "$sketch_g" "$port_link_g" "$remote_host_g"
elif [[ "x$1" == "xh" ]]; then
    build "$sketch_h" "$port_link_h" "$remote_host_h"
elif [[ "x$1" == "xx" ]]; then
    build "$sketch_c" "$port_link_c" "$remote_host_c"
    build "$sketch_l" "$port_link_l" "$remote_host_l"
    build "$sketch_r" "$port_link_r" "$remote_host_r"
    build "$sketch_w" "$port_link_w" "$remote_host_w"
    build "$sketch_a" "$port_link_a" "$remote_host_a"
    build "$sketch_g" "$port_link_g" "$remote_host_g"
elif [[ "x$1" == "xm" ]]; then
    build "$sketch_c" "$port_link_c" "$remote_host_c"
    build "$sketch_l" "$port_link_l" "$remote_host_l"
    build "$sketch_r" "$port_link_r" "$remote_host_r"
    build "$sketch_w" "$port_link_w" "$remote_host_w"
else
    echo "error:  unrecognized option"
    exit 1
fi

# wait for udev to plug /dev/ttyACMw? back in
# or else it does not appear in wg-arduino-stty.sh
#sleep 5
## $RESET_PORTS
#ssh weathers $RESET_PORTS
#ssh weatherw $RESET_PORTS
#ssh weatherg $RESET_PORTS

exit 0

# eee eof
