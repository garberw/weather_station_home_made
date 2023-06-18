#!/bin/bash

SCRIPT_HOME=$(dirname $(readlink -f $0))

LOG="junk.eraseme.serial_numbers.log"

rm "$LOG"

echo "ACM0" | tee -a "$LOG"
udevadm info -a -n /dev/ttyACM0 | grep serial | grep -v ':'  | tee -a "$LOG"

echo "ACM1" | tee -a "$LOG"
udevadm info -a -n /dev/ttyACM1 | grep serial | grep -v ':'  | tee -a "$LOG"

echo "ACM2" | tee -a "$LOG"
udevadm info -a -n /dev/ttyACM2 | grep serial | grep -v ':'  | tee -a "$LOG"

echo "ACM3" | tee -a "$LOG"
udevadm info -a -n /dev/ttyACM3 | grep serial | grep -v ':'  | tee -a "$LOG"

echo "ACM4" | tee -a "$LOG"
udevadm info -a -n /dev/ttyACM4 | grep serial | grep -v ':'  | tee -a "$LOG"




# pipe into less and search for "ATTRS{serial}"
# paste serial number into "etc__udev__rules.d__99-usbserial-arduino-garberw.rules"


# eee eof
