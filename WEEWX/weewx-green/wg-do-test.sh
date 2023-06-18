#!/bin/bash

SCRIPT_HOME=$(dirname $(readlink -f $0))

# PYTHONPATH=/usr/share/weewx python /usr/share/weewx/user/weewx_green.py --port=/dev/ttyUSBwi
PYTHONPATH=/usr/share/weewx python /usr/share/weewx/user/weewx_green.py

# eee eof
