#!/bin/bash

SCRIPT_HOME=$(dirname $(readlink -f $0))

# PYTHONPATH=/usr/share/weewx python /usr/share/weewx/user/weewx_wmod.py --port=/dev/ttyUSBww

PYTHONPATH=/usr/share/weewx python /usr/share/weewx/user/weewx_wmod.py --port=/dev/ttyACMww --debug

# eee eof
