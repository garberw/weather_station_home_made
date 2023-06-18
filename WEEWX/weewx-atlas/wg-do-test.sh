#!/bin/bash

SCRIPT_HOME=$(dirname $(readlink -f $0))

DATE=`date -Is`

EXEC="/usr/share/weewx/user/weewx_atlas.py"

LOG="$SCRIPT_HOME/LOG/run5_${DATE}.log"

NEW_PYTHONPATH=$PYTHONPATH:/usr/share/weewx/

CONFIG=--config="/etc/weewx/weewx_atlas.conf"


PYTHONPATH=$NEW_PYTHONPATH python "$EXEC" $CONFIG  2>&1 | tee "$LOG"


# eee eof
