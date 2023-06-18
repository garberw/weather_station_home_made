#!/bin/bash

SCRIPT_HOME=$(dirname $(readlink -f $0))

# udevadm info -a -n /dev/ttyACM0


cp $SCRIPT_HOME/etc__udev__rules.d__99-usbserial-arduino-garberw.rules /etc/udev/rules.d/99-usbserial-arduino-garberw.rules

udevadm control --reload-rules

udevadm trigger

# eee eof
