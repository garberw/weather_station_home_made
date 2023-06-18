#!/bin/bash

# set -x

# exit when any command does not return 0
set -e
source /etc/default/wshm
source $GIT_HOME/HUB/CONFIG

checkhost $DESKTOP_HOST

checkroot

function raspi_setup() {
    TARGET_SUFFIX="$1"
    TARGET_HOST="weather${1}"
    LOC="on TARGET_HOST=$TARGET_HOST"
    
    echo "$PROMPT $LOC install power off button service"
    ssh $TARGET_HOST $GIT_HOME/raspi_user/pi-power-button-master/script.systemd/install.sh
    
    if [[ $1 == 'e' ]]; then
	echo "$PROMPT $LOC install raspi service and corresponding systemd service"
        echo "$PROMPT weathere service monitors sensors for green weather station outdoors (e=exterior);"
	ssh $TARGET_HOST $GIT_HOME/raspi_user/weather${TARGET_SUFFIX}_python_service/wg-install-program.sh
	ssh $TARGET_HOST $GIT_HOME/raspi_user/weather${TARGET_SUFFIX}_python_service/systemd/wg-install-service.sh
    fi
    if [[ $1 == 'g' ]]; then
	echo "$PROMPT $LOC install raspi service and corresponding systemd service"
        echo "$PROMPT weatherg service monitors sensors for green weather station indoors;"
	ssh $TARGET_HOST $GIT_HOME/raspi_user/weather${TARGET_SUFFIX}_python_service/wg-install-program.sh
	ssh $TARGET_HOST $GIT_HOME/raspi_user/weather${TARGET_SUFFIX}_python_service/systemd/wg-install-service.sh
    fi

    echo "$PROMPT $LOC install flags for arduino linker"
    # uncomment to clobber existing flags
    # ssh $TARGET_HOST $GIT_HOME/raspi_user/flags_arduino/install.sh
    # ...or...
    echo "$PROMPT log on to $TARGET_HOST   ...and..."
    echo "$PROMPT hand edit to not overwrite existing data ...or..."
    echo "$PROMPT run $GIT_HOME/raspi_user/flags_arduino/install.sh"
}

LOC="on DESKTOP_HOST=$DESKTOP_HOST"

echo "$PROMPT $LOC setup link in ~/bin for wg-weather-gui.py"
# this is for development; not a great way to install;
su - $USER -c "$LN $GIT_HOME/raspi_user/wg-weather-gui/wg-weather-gui.py $GIT_HOME/bin/"

echo "$PROMPT $LOC install GIT_HOME/Arduino to ARDUINO_HOME"
$GIT_HOME/bin/wg-WEATHER-INSTALL-git-to-arduino-home.sh

echo "$PROMPT $LOC copy ARDUINO_HOME to each raspi"
$GIT_HOME/bin/wg-WEATHER-push-arduino-home-from-desktop-to-raspi.sh

echo "$PROMPT $LOC copy GIT_HOME to each raspi"
$GIT_HOME/bin/wg-WEATHER-push-git-from-desktop-to-raspi.sh

raspi_setup s
raspi_setup e
raspi_setup g
raspi_setup w

echo "$PROMPT $LOC copy bin to each raspi"
$GIT_HOME/bin/wg-WEATHER-push-bin-from-git-to-desktop-and-raspi.sh

echo "$PROMPT $LOC copy bashrc to each raspi"
$GIT_HOME/bin/wg-WEATHER-push-bashrc-from-git-to-desktop-and-raspi.sh

echo "$PROMPT $LOC setup weewx"
$GIT_HOME/bin/wg-WEATHER-setup-weewx.sh

echo "$PROMPT $LOC when adding new arduinos run $GIT_HOME/bin/wg-WEATHER-setup-udev.sh"

# eee eof
